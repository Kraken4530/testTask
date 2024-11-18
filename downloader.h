#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <thread>
#include <mutex>
#include <curl/curl.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <regex>

std::string getCurrentTime();

void logEvent(const std::string &message);

std::string ExtractFilename(const std::string &contentDisposition);

bool downloadFile(const std::string &url, const std::string &outputPath);

void downloadMultipleFiles(const std::vector<std::string> &urls,
                           const std::string &outputDir,
                           size_t maxThreads);

#endif // DOWNLOADER_H