#pragma once

#include "download/DownloadJob.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dlmanager {

struct DownloadStats {
    std::size_t totalJobs{};
    std::size_t queuedCount{};
    std::size_t activeCount{};
    std::size_t pausedCount{};
    std::size_t completedCount{};
    std::size_t cancelledCount{};
    std::size_t totalBytesDownloaded{};
    std::size_t totalBytesExpected{};
};

class DownloadManager {
public:
    explicit DownloadManager(std::string dbFile = "data/downloads.db",
                             std::string outputDir = "data");

    bool load();
    bool save() const;

    DownloadJob& add(std::string url, std::size_t totalBytes = 1048576, std::string customFileName = "");
    DownloadJob* find(int id);
    const DownloadJob* find(int id) const;
    bool remove(int id);

    bool start(int id);
    bool pause(int id);
    bool resume(int id);
    bool cancel(int id);
    bool retry(int id);

    // Advances active/selected download by chunkBytes (or completes it if finishAll == true)
    bool processDownload(int id, std::size_t chunkBytes = 0, bool finishAll = true);
    std::size_t startAllQueued();

    std::vector<const DownloadJob*> filterByStatus(std::string_view statusFilter) const;
    std::vector<const DownloadJob*> search(std::string_view query) const;
    DownloadStats computeStats() const;

    const std::vector<DownloadJob>& jobs() const { return jobs_; }
    const std::string& dbFile() const { return dbFile_; }
    const std::string& outputDir() const { return outputDir_; }

private:
    bool writeDestinationArtifact(const DownloadJob& job) const;

    std::string dbFile_;
    std::string outputDir_;
    std::vector<DownloadJob> jobs_;
    int nextId_{1};
};

} // namespace dlmanager

