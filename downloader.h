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
#include <curl/curl.h>

extern std::mutex consoleMutex;
extern std::mutex dirFileNamesMutex;
extern std::set<std::string> dirFileNames;

std::string getCurrentTime();

void logEvent(const std::string &message);

size_t WriteToFile(void *ptr, size_t size, size_t nmemb, FILE *stream);

size_t HeaderCallback(char *buffer, size_t size, size_t nitems, std::string *headerData);

std::string ExtractFilename(const std::string &contentDisposition);

std::string ExtractFilenameFromUrl(const std::string &url);

std::string getFileNameWithNumber(std::string startFileName);

bool downloadFile(const std::string &url, const std::string &outputDir);

void downloadMultipleFiles(const std::vector<std::string> &urls, const std::string &outputDir, size_t maxThreads);

#endif // DOWNLOADER_H
