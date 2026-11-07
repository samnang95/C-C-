#include "download/DownloadJob.hpp"
#include "download/DownloadManager.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>

int main() {
    using namespace dlmanager;

    // 1. URL filename extraction, size formatting, and parsing
    assert(extractFileNameFromUrl("https://example.com/files/archive.tar.gz?token=123", 1) == "archive.tar.gz");
    assert(extractFileNameFromUrl("https://example.com/", 3) == "download_3.bin");
    assert(formatBytes(512) == "512 B");
    assert(formatBytes(1024 * 1024) == "1.00 MB");

    std::size_t parsedBytes = 0;
    assert(parseSizeBytes("2MB", parsedBytes) && parsedBytes == 2 * 1024 * 1024);
    assert(parseSizeBytes("500KB", parsedBytes) && parsedBytes == 500 * 1024);
    assert(!parseSizeBytes("-10MB", parsedBytes));

    // 2. DownloadManager queue lifecycle, progress, stats, and persistence
    const std::string testDb = "/tmp/test_download_manager.db";
    std::remove(testDb.c_str());

    {
        DownloadManager mgr(testDb, "/tmp");

        const auto& j1 = mgr.add("https://mirror.example.org/ubuntu.iso", 1000, "ubuntu.iso");
        assert(j1.id == 1);
        assert(j1.status == DownloadStatus::Queued);

        const auto& j2 = mgr.add("https://mirror.example.org/docs.pdf", 400);
        assert(j2.id == 2);
        assert(j2.fileName == "docs.pdf");

        // Advance job 1 by 250 bytes (25%)
        assert(mgr.processDownload(1, 250, false));
        assert(mgr.find(1)->status == DownloadStatus::Downloading);
        assert(std::abs(mgr.find(1)->progressPercent() - 25.0) < 1e-6);

        // Pause and resume job 1
        assert(mgr.pause(1));
        assert(mgr.find(1)->status == DownloadStatus::Paused);
        assert(mgr.resume(1));
        assert(mgr.find(1)->status == DownloadStatus::Downloading);

        // Complete job 1
        assert(mgr.processDownload(1, 0, true));
        assert(mgr.find(1)->status == DownloadStatus::Completed);
        assert(std::abs(mgr.find(1)->progressPercent() - 100.0) < 1e-6);

        // Cancel job 2, then retry
        assert(mgr.cancel(2));
        assert(mgr.find(2)->status == DownloadStatus::Cancelled);
        assert(mgr.retry(2));
        assert(mgr.find(2)->status == DownloadStatus::Queued);

        const auto stats = mgr.computeStats();
        assert(stats.totalJobs == 2);
        assert(stats.completedCount == 1);
        assert(stats.queuedCount == 1);
        assert(stats.totalBytesDownloaded == 1000);

        assert(mgr.save());
    }

    {
        DownloadManager loaded(testDb, "/tmp");
        assert(loaded.load());
        assert(loaded.jobs().size() == 2);
        assert(loaded.find(1)->status == DownloadStatus::Completed);
        assert(loaded.find(2)->status == DownloadStatus::Queued);

        assert(loaded.remove(1));
        assert(loaded.jobs().size() == 1);
    }

    std::remove(testDb.c_str());
    std::remove("/tmp/ubuntu.iso");
    std::cout << "All download manager unit tests passed!\n";
    return 0;
}

