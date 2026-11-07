#pragma once

#include "download/DownloadJob.hpp"
#include "download/DownloadManager.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dlmanager {

void printHelp();
std::string renderProgressBar(double percent, std::size_t width = 20);
void printJob(const DownloadJob& job);
void listJobs(const std::vector<const DownloadJob*>& jobs);
void listJobs(const std::vector<DownloadJob>& jobs);
void printStats(const DownloadStats& stats);

bool parseId(std::string_view text, int& id);
bool parseSizeBytes(std::string_view text, std::size_t& bytes);
std::string prompt(std::string_view label);

void addDownloadInteractive(DownloadManager& manager, std::string_view inlineUrl = "");
void runDownloadAnimation(DownloadManager& manager, int id);

} // namespace dlmanager

