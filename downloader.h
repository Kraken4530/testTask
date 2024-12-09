#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <string>
#include <vector>
#include <mutex>
#include <set>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <regex>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>

extern std::mutex consoleMutex;
extern std::mutex fileMutex;

std::string getCurrentTime();

void logEvent(const std::string &message);

std::string replaceInvalidFilenameChars(const std::string &filename);

std::string extractFilenameFromUrl(const std::string &url);

std::string extractFilename(const std::string &contentDisposition);

std::string generateUniqueFilename(const std::string &outputDir, const std::string &filename);

bool downloadFile(const std::string &url, const std::string &outputDir);

void downloadMultipleFiles(const std::vector<std::string> &urls, const std::string &outputDir, size_t maxThreads);

#endif // DOWNLOADER_H
