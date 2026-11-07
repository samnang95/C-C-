#include "download/DownloadJob.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace dlmanager {

double DownloadJob::progressPercent() const {
    if (totalBytes == 0) {
        return status == DownloadStatus::Completed ? 100.0 : 0.0;
    }
    const double pct = (static_cast<double>(downloadedBytes) / static_cast<double>(totalBytes)) * 100.0;
    return std::min(100.0, pct);
}

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string lowercase(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool containsIgnoreCase(std::string_view value, std::string_view queryLower) {
    return lowercase(value).find(queryLower) != std::string::npos;
}

std::string statusName(DownloadStatus status) {
    switch (status) {
        case DownloadStatus::Queued:      return "queued";
        case DownloadStatus::Downloading: return "downloading";
        case DownloadStatus::Paused:      return "paused";
        case DownloadStatus::Completed:   return "completed";
        case DownloadStatus::Cancelled:   return "cancelled";
        case DownloadStatus::Failed:      return "failed";
    }
    return "queued";
}

DownloadStatus parseStatus(std::string_view text) {
    const auto v = lowercase(trim(text));
    if (v == "downloading") return DownloadStatus::Downloading;
    if (v == "paused")      return DownloadStatus::Paused;
    if (v == "completed")   return DownloadStatus::Completed;
    if (v == "cancelled")   return DownloadStatus::Cancelled;
    if (v == "failed")      return DownloadStatus::Failed;
    return DownloadStatus::Queued;
}

std::string extractFileNameFromUrl(std::string_view url, int fallbackId) {
    auto cleaned = trim(url);
    const auto queryPos = cleaned.find_first_of("?#");
    if (queryPos != std::string_view::npos) {
        cleaned = cleaned.substr(0, queryPos);
    }

    const auto schemePos = cleaned.find("://");
    const std::size_t pathStart = (schemePos == std::string_view::npos)
                                      ? cleaned.find('/')
                                      : cleaned.find('/', schemePos + 3);
    if (pathStart == std::string_view::npos) {
        return "download_" + std::to_string(fallbackId) + ".bin";
    }

    std::string_view path = cleaned.substr(pathStart + 1);
    while (!path.empty() && path.back() == '/') {
        path.remove_suffix(1);
    }
    if (path.empty()) {
        return "download_" + std::to_string(fallbackId) + ".bin";
    }

    const auto slashPos = path.find_last_of('/');
    std::string_view candidate = (slashPos == std::string_view::npos)
                                     ? path
                                     : path.substr(slashPos + 1);

    std::string safeName;
    for (const char c : candidate) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_') {
            safeName += c;
        } else {
            safeName += '_';
        }
    }
    return safeName.empty() ? ("download_" + std::to_string(fallbackId) + ".bin") : safeName;
}

std::string formatBytes(std::size_t bytes) {
    static const char* kUnits[] = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes);
    std::size_t unitIdx = 0;
    while (value >= 1024.0 && unitIdx < 3) {
        value /= 1024.0;
        ++unitIdx;
    }

    std::ostringstream out;
    if (unitIdx == 0) {
        out << bytes << " B";
    } else {
        out << std::fixed << std::setprecision(2) << value << ' ' << kUnits[unitIdx];
    }
    return out.str();
}

} // namespace dlmanager
