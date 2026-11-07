#include "download/DownloadManager.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>

namespace dlmanager {

namespace {

std::string escapeField(std::string_view value) {
    std::string result;
    for (const char c : value) {
        if (c == '\\') result += "\\\\";
        else if (c == '\t') result += "\\t";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

std::string unescapeField(std::string_view value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            result += (next == 't') ? '\t' : ((next == 'n') ? '\n' : next);
        } else {
            result += value[i];
        }
    }
    return result;
}

std::vector<std::string> splitFields(std::string_view line) {
    std::vector<std::string> fields;
    std::string field;
    bool escaped = false;
    for (const char c : line) {
        if (escaped) {
            field += '\\';
            field += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '\t') {
            fields.push_back(unescapeField(field));
            field.clear();
        } else {
            field += c;
        }
    }
    if (escaped) field += '\\';
    fields.push_back(unescapeField(field));
    return fields;
}

template <typename IntType>
bool parseInteger(std::string_view text, IntType& out) {
    auto res = std::from_chars(text.data(), text.data() + text.size(), out);
    return res.ec == std::errc() && res.ptr == text.data() + text.size();
}

} // namespace

DownloadManager::DownloadManager(std::string dbFile, std::string outputDir)
    : dbFile_(std::move(dbFile)), outputDir_(std::move(outputDir)) {}

bool DownloadManager::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitFields(line);
        if (fields.size() != 7) continue;

        int id = 0;
        std::size_t totalBytes = 0;
        std::size_t downloadedBytes = 0;
        if (!parseInteger(fields[0], id) || id < 1) continue;
        if (!parseInteger(fields[4], totalBytes)) continue;
        if (!parseInteger(fields[5], downloadedBytes)) continue;

        if (fields[1].empty() || fields[2].empty()) continue;

        DownloadJob job{
            id,
            fields[1],
            fields[2],
            fields[3],
            totalBytes,
            std::min(downloadedBytes, totalBytes),
            parseStatus(fields[6])
        };

        jobs_.push_back(std::move(job));
        nextId_ = std::max(nextId_, id + 1);
    }
    return true;
}

bool DownloadManager::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    for (const auto& job : jobs_) {
        output << job.id << '\t'
               << escapeField(job.url) << '\t'
               << escapeField(job.fileName) << '\t'
               << escapeField(job.destinationPath) << '\t'
               << job.totalBytes << '\t'
               << job.downloadedBytes << '\t'
               << statusName(job.status) << '\n';
    }
    return static_cast<bool>(output);
}

DownloadJob& DownloadManager::add(std::string url, std::size_t totalBytes, std::string customFileName) {
    const int id = nextId_++;
    if (totalBytes == 0) totalBytes = 1048576; // 1 MB default

    std::string fileName = std::string(trim(customFileName));
    if (fileName.empty()) {
        fileName = extractFileNameFromUrl(url, id);
    }

    std::string destPath = outputDir_.empty() ? fileName : (outputDir_ + "/" + fileName);

    DownloadJob job{
        id,
        std::string(trim(url)),
        std::move(fileName),
        std::move(destPath),
        totalBytes,
        0,
        DownloadStatus::Queued
    };

    jobs_.push_back(std::move(job));
    return jobs_.back();
}

DownloadJob* DownloadManager::find(int id) {
    for (auto& job : jobs_) {
        if (job.id == id) return &job;
    }
    return nullptr;
}

const DownloadJob* DownloadManager::find(int id) const {
    for (const auto& job : jobs_) {
        if (job.id == id) return &job;
    }
    return nullptr;
}

bool DownloadManager::remove(int id) {
    const auto oldSize = jobs_.size();
    jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(),
                               [id](const DownloadJob& j) { return j.id == id; }),
                jobs_.end());
    return jobs_.size() != oldSize;
}

bool DownloadManager::start(int id) {
    DownloadJob* job = find(id);
    if (!job) return false;
    if (job->status == DownloadStatus::Completed) return false;
    job->status = DownloadStatus::Downloading;
    return true;
}

bool DownloadManager::pause(int id) {
    DownloadJob* job = find(id);
    if (!job || job->status != DownloadStatus::Downloading) return false;
    job->status = DownloadStatus::Paused;
    return true;
}

bool DownloadManager::resume(int id) {
    DownloadJob* job = find(id);
    if (!job || (job->status != DownloadStatus::Paused && job->status != DownloadStatus::Queued)) {
        return false;
    }
    job->status = DownloadStatus::Downloading;
    return true;
}

bool DownloadManager::cancel(int id) {
    DownloadJob* job = find(id);
    if (!job || job->status == DownloadStatus::Completed) return false;
    job->status = DownloadStatus::Cancelled;
    return true;
}

bool DownloadManager::retry(int id) {
    DownloadJob* job = find(id);
    if (!job) return false;
    job->downloadedBytes = 0;
    job->status = DownloadStatus::Queued;
    return true;
}

bool DownloadManager::writeDestinationArtifact(const DownloadJob& job) const {
    if (job.destinationPath.empty()) return true;
    std::ofstream out(job.destinationPath, std::ios::trunc);
    if (!out) return false;
    out << "Downloaded from: " << job.url << '\n'
        << "File: " << job.fileName << '\n'
        << "Size: " << job.totalBytes << " bytes\n";
    return static_cast<bool>(out);
}

bool DownloadManager::processDownload(int id, std::size_t chunkBytes, bool finishAll) {
    DownloadJob* job = find(id);
    if (!job) return false;
    if (job->status == DownloadStatus::Cancelled) return false;

    job->status = DownloadStatus::Downloading;
    if (finishAll || chunkBytes == 0 || job->downloadedBytes + chunkBytes >= job->totalBytes) {
        job->downloadedBytes = job->totalBytes;
        job->status = DownloadStatus::Completed;
        writeDestinationArtifact(*job);
    } else {
        job->downloadedBytes += chunkBytes;
    }
    return true;
}

std::size_t DownloadManager::startAllQueued() {
    std::size_t completed = 0;
    for (auto& job : jobs_) {
        if (job.status == DownloadStatus::Queued || job.status == DownloadStatus::Downloading ||
            job.status == DownloadStatus::Paused) {
            processDownload(job.id, 0, true);
            ++completed;
        }
    }
    return completed;
}

std::vector<const DownloadJob*> DownloadManager::filterByStatus(std::string_view statusFilter) const {
    const auto target = lowercase(trim(statusFilter));
    std::vector<const DownloadJob*> result;
    for (const auto& job : jobs_) {
        if (target.empty() || statusName(job.status) == target) {
            result.push_back(&job);
        }
    }
    return result;
}

std::vector<const DownloadJob*> DownloadManager::search(std::string_view query) const {
    const auto queryLower = lowercase(trim(query));
    std::vector<const DownloadJob*> matches;
    if (queryLower.empty()) return matches;

    for (const auto& job : jobs_) {
        if (containsIgnoreCase(job.url, queryLower) ||
            containsIgnoreCase(job.fileName, queryLower) ||
            containsIgnoreCase(statusName(job.status), queryLower)) {
            matches.push_back(&job);
        }
    }
    return matches;
}

DownloadStats DownloadManager::computeStats() const {
    DownloadStats stats;
    stats.totalJobs = jobs_.size();
    for (const auto& job : jobs_) {
        stats.totalBytesDownloaded += job.downloadedBytes;
        stats.totalBytesExpected += job.totalBytes;
        switch (job.status) {
            case DownloadStatus::Queued:      ++stats.queuedCount; break;
            case DownloadStatus::Downloading: ++stats.activeCount; break;
            case DownloadStatus::Paused:      ++stats.pausedCount; break;
            case DownloadStatus::Completed:   ++stats.completedCount; break;
            case DownloadStatus::Cancelled:
            case DownloadStatus::Failed:      ++stats.cancelledCount; break;
        }
    }
    return stats;
}

} // namespace dlmanager

