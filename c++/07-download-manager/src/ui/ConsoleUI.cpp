#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

namespace dlmanager {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add [url]             Queue a new download job\n"
              << "  list [status]         List downloads (queued/downloading/paused/completed/cancelled)\n"
              << "  view [id]             Show detailed download information\n"
              << "  start [id|all]        Start and run download(s) with progress bar\n"
              << "  step [id]             Advance download progress by 25%\n"
              << "  pause [id]            Pause an active download\n"
              << "  resume [id]           Resume a paused download\n"
              << "  cancel [id]           Cancel a download\n"
              << "  retry [id]            Reset and re-queue a download\n"
              << "  remove [id]           Remove a download from the list\n"
              << "  search <text>         Search downloads by URL, filename, or status\n"
              << "  stats                 Show overall download queue statistics\n"
              << "  clear / reset         Clear the terminal screen\n"
              << "  help                  Show this help\n"
              << "  quit                  Save queue and exit\n\n";
}

std::string renderProgressBar(double percent, std::size_t width) {
    const double clamped = std::max(0.0, std::min(100.0, percent));
    const std::size_t filled = static_cast<std::size_t>(std::round((clamped / 100.0) * static_cast<double>(width)));
    std::string bar = "[";
    for (std::size_t i = 0; i < width; ++i) {
        bar += (i < filled) ? '#' : '-';
    }
    bar += "]";
    return bar;
}

void printJob(const DownloadJob& job) {
    std::cout << std::fixed << std::setprecision(1)
              << "\nDownload #" << job.id << " — " << job.fileName << '\n'
              << "  URL:         " << job.url << '\n'
              << "  Destination: " << job.destinationPath << '\n'
              << "  Status:      " << statusName(job.status) << '\n'
              << "  Progress:    " << renderProgressBar(job.progressPercent(), 24) << ' '
              << job.progressPercent() << "% (" << formatBytes(job.downloadedBytes)
              << " / " << formatBytes(job.totalBytes) << ")\n\n";
}

void listJobs(const std::vector<const DownloadJob*>& jobs) {
    if (jobs.empty()) {
        std::cout << "\nNo matching downloads.\n\n";
        return;
    }

    std::cout << "\n" << std::left << std::setw(4) << "ID"
              << std::setw(13) << "Status"
              << std::setw(24) << "File"
              << std::setw(24) << "Progress"
              << std::setw(20) << "Transferred"
              << "URL\n"
              << std::string(100, '-') << '\n';

    std::cout << std::fixed << std::setprecision(0);
    for (const auto* job : jobs) {
        if (!job) continue;
        const std::string bar = renderProgressBar(job->progressPercent(), 14) + " " +
                                std::to_string(static_cast<int>(std::round(job->progressPercent()))) + "%";
        const std::string transferred = formatBytes(job->downloadedBytes) + "/" + formatBytes(job->totalBytes);

        std::cout << std::left << std::setw(4) << job->id
                  << std::setw(13) << statusName(job->status)
                  << std::setw(24) << job->fileName.substr(0, 23)
                  << std::setw(24) << bar
                  << std::setw(20) << transferred
                  << job->url.substr(0, 32) << '\n';
    }
    std::cout << '\n';
}

void listJobs(const std::vector<DownloadJob>& jobs) {
    std::vector<const DownloadJob*> ptrs;
    ptrs.reserve(jobs.size());
    for (const auto& job : jobs) {
        ptrs.push_back(&job);
    }
    listJobs(ptrs);
}

void printStats(const DownloadStats& stats) {
    std::cout << "\nDownload Queue Summary:\n"
              << "  Total jobs:       " << stats.totalJobs << '\n'
              << "  Queued:           " << stats.queuedCount << '\n'
              << "  Downloading:      " << stats.activeCount << '\n'
              << "  Paused:           " << stats.pausedCount << '\n'
              << "  Completed:        " << stats.completedCount << '\n'
              << "  Cancelled/Failed: " << stats.cancelledCount << '\n'
              << "  Data transferred: " << formatBytes(stats.totalBytesDownloaded)
              << " / " << formatBytes(stats.totalBytesExpected) << "\n\n";
}

bool parseId(std::string_view text, int& id) {
    auto trimmed = trim(text);
    if (trimmed.empty()) return false;

    int parsed = 0;
    auto res = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), parsed);
    if (res.ec != std::errc() || res.ptr != trimmed.data() + trimmed.size() || parsed <= 0) {
        return false;
    }
    id = parsed;
    return true;
}

bool parseSizeBytes(std::string_view text, std::size_t& bytes) {
    auto trimmed = lowercase(trim(text));
    if (trimmed.empty()) return false;

    double multiplier = 1.0;
    if (trimmed.size() > 2 && trimmed.substr(trimmed.size() - 2) == "kb") {
        multiplier = 1024.0;
        trimmed = std::string(trim(trimmed.substr(0, trimmed.size() - 2)));
    } else if (trimmed.size() > 2 && trimmed.substr(trimmed.size() - 2) == "mb") {
        multiplier = 1024.0 * 1024.0;
        trimmed = std::string(trim(trimmed.substr(0, trimmed.size() - 2)));
    } else if (trimmed.size() > 2 && trimmed.substr(trimmed.size() - 2) == "gb") {
        multiplier = 1024.0 * 1024.0 * 1024.0;
        trimmed = std::string(trim(trimmed.substr(0, trimmed.size() - 2)));
    } else if (trimmed.size() > 1 && trimmed.back() == 'b') {
        trimmed = std::string(trim(trimmed.substr(0, trimmed.size() - 1)));
    }

    char* endPtr = nullptr;
    const double val = std::strtod(trimmed.c_str(), &endPtr);
    if (endPtr == trimmed.c_str() || *endPtr != '\0' || val <= 0.0) {
        return false;
    }
    bytes = static_cast<std::size_t>(val * multiplier);
    return bytes > 0;
}

std::string prompt(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) return "";
    return std::string(trim(value));
}

void addDownloadInteractive(DownloadManager& manager, std::string_view inlineUrl) {
    std::string url = std::string(trim(inlineUrl));
    while (url.empty() && std::cin) {
        url = prompt("Download URL: ");
        if (!std::cin) return;
        if (url.empty()) {
            std::cout << "A URL is required.\n";
        }
    }

    const auto customName = prompt("Save as filename (optional, Enter for auto): ");
    if (!std::cin) return;

    std::size_t sizeBytes = 5 * 1024 * 1024; // 5 MB default
    while (std::cin) {
        const auto rawSize = prompt("Estimated size e.g. 5MB, 512KB (5MB): ");
        if (!std::cin) return;
        if (rawSize.empty()) break;
        if (parseSizeBytes(rawSize, sizeBytes)) break;
        std::cout << "Please enter a positive size such as 2MB, 500KB, or 1048576.\n";
    }

    const auto& added = manager.add(std::move(url), sizeBytes, customName);
    std::cout << "Queued download #" << added.id << " (" << added.fileName
              << ", " << formatBytes(added.totalBytes) << ").\n";
}

void runDownloadAnimation(DownloadManager& manager, int id) {
    DownloadJob* job = manager.find(id);
    if (!job) {
        std::cout << "Download not found.\n";
        return;
    }
    if (job->status == DownloadStatus::Completed) {
        std::cout << "Download #" << id << " is already completed.\n";
        return;
    }

    job->status = DownloadStatus::Downloading;
    const std::size_t stepBytes = std::max<std::size_t>(1, job->totalBytes / 10);

    while (job->downloadedBytes < job->totalBytes) {
        manager.processDownload(id, stepBytes, false);
        std::cout << "\r  Downloading " << job->fileName << " "
                  << renderProgressBar(job->progressPercent(), 24) << " "
                  << std::fixed << std::setprecision(0) << job->progressPercent() << "% ("
                  << formatBytes(job->downloadedBytes) << "/" << formatBytes(job->totalBytes) << ")"
                  << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(45));
    }
    std::cout << "\nCompleted download #" << job->id << " -> " << job->destinationPath << '\n';
}

} // namespace dlmanager

