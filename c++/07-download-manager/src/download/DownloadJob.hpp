#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace dlmanager {

enum class DownloadStatus {
    Queued,
    Downloading,
    Paused,
    Completed,
    Cancelled,
    Failed
};

struct DownloadJob {
    int id{};
    std::string url;
    std::string fileName;
    std::string destinationPath;
    std::size_t totalBytes{};
    std::size_t downloadedBytes{};
    DownloadStatus status{DownloadStatus::Queued};

    double progressPercent() const;
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);

std::string statusName(DownloadStatus status);
DownloadStatus parseStatus(std::string_view text);
std::string extractFileNameFromUrl(std::string_view url, int fallbackId = 1);
std::string formatBytes(std::size_t bytes);

} // namespace dlmanager

