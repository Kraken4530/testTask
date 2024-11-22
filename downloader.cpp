#include "downloader.h"

std::mutex consoleMutex;
std::mutex dirFileNamesMutex;
std::set<std::string> dirFileNames;

// Получение текущего времени в формате строки с миллисекундами
std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

// Запись события в консоль с учетом синхронизации потоков
void logEvent(const std::string &message)
{
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "[" << getCurrentTime() << "] " << message << std::endl;
}

// Запись данных в файл (CURL callback)
size_t WriteToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

// Callback для обработки заголовков
size_t HeaderCallback(char *buffer, size_t size, size_t nitems, std::string *headerData)
{
    size_t totalSize = size * nitems;
    std::string header(buffer, totalSize);

    if (header.find("Content-Disposition:") != std::string::npos)
    {
        *headerData = header;
    }

    return totalSize;
}

// Извлечение имени файла из заголовка Content-Disposition
std::string ExtractFilename(const std::string &contentDisposition)
{
    std::string filenameKey = "filename=";
    size_t pos = contentDisposition.find(filenameKey);
    if (pos != std::string::npos)
    {
        pos += filenameKey.length();
        if (contentDisposition[pos] == '\"')
        {
            pos++;
            size_t end = contentDisposition.find('\"', pos);
            if (end != std::string::npos)
            {
                return contentDisposition.substr(pos, end - pos);
            }
        }
        else
        {
            size_t end = contentDisposition.find(';', pos);
            return contentDisposition.substr(pos, end - pos);
        }
    }
    return "";
}

// Извлечение имени файла из URL
std::string ExtractFilenameFromUrl(const std::string &url)
{
    size_t lastSlash = url.find_last_of('/');
    std::string filename = (lastSlash != std::string::npos) ? url.substr(lastSlash + 1) : "downloaded_file";

    // Замена недопустимых символов в имени файла на '_'
    std::regex invalidChars("[^a-zA-Z0-9._-]");
    filename = std::regex_replace(filename, invalidChars, "_");

    return filename;
}

// Генерация уникального имени файла с добавлением номера (если требуется)
std::string getFileNameWithNumber(std::string startFileName)
{
    std::lock_guard<std::mutex> lock(dirFileNamesMutex);

    size_t lastDotPos = startFileName.rfind('.');
    if (lastDotPos == std::string::npos)
        lastDotPos = startFileName.size();
    std::string name = startFileName.substr(0, lastDotPos);
    std::string extension = startFileName.substr(lastDotPos, startFileName.size());

    int cur_ind = 0;
    while (dirFileNames.find(name + (cur_ind ? "(" + std::to_string(cur_ind) + ")" : "") + extension) != dirFileNames.end())
        cur_ind++;

    std::string uniqueName = name + (cur_ind ? "(" + std::to_string(cur_ind) + ")" : "") + extension;
    dirFileNames.insert(uniqueName);
    return uniqueName;
}

// Загрузка файла по указанному URL в указанную директорию
bool downloadFile(const std::string &url, const std::string &outputDir)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        logEvent("Error: Unable to initialize CURL for URL: " + url);
        return false;
    }

    std::string headerData;
    bool success = false;

    logEvent("Start downloading: " + url);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

    // Генерация имени файла из URL
    std::string filename = ExtractFilenameFromUrl(url);
    filename = getFileNameWithNumber(filename);

    std::string outputPath = outputDir + "/" + filename;
    try
    {
        std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        logEvent("Error: Unable to create directories for path: " + outputPath + " with error: " + e.what());
        curl_easy_cleanup(curl);
        return false;
    }
    FILE *file = fopen(outputPath.c_str(), "wb");
    if (!file)
    {
        logEvent("Error: Unable to create file: " + outputPath);
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    // Выполнение запроса
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        logEvent("Error: CURL failed for URL: " + url + " with error: " + curl_easy_strerror(res));
    }
    else
    {
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode != 200)
        {
            logEvent("Warning: Server responded with code " + std::to_string(responseCode) + " for URL: " + url);
        }
        else
        {
            // Проверка, было ли извлечено имя файла из заголовков
            std::string contentDispositionFilename = ExtractFilename(headerData);
            if (!contentDispositionFilename.empty())
            {
                {
                    std::lock_guard<std::mutex> lock(dirFileNamesMutex);
                    dirFileNames.erase(filename);
                }

                contentDispositionFilename = getFileNameWithNumber(contentDispositionFilename);
                std::string newOutputPath = outputDir + "/" + contentDispositionFilename;
                if (std::rename(outputPath.c_str(), newOutputPath.c_str()) == 0)
                {
                    outputPath = newOutputPath;
                }

                {
                    std::lock_guard<std::mutex> lock(dirFileNamesMutex);
                    dirFileNames.insert(contentDispositionFilename);
                }
            }

            logEvent("Successfully downloaded: " + outputPath);
            success = true;
        }
    }

    fclose(file);

    if (!success)
    {
        {
            std::lock_guard<std::mutex> lock(dirFileNamesMutex);
            dirFileNames.erase(filename);
        }
        if (std::remove(outputPath.c_str()) == 0)
        {
            logEvent("Info: Incomplete file removed: " + outputPath);
        }
        else
        {
            logEvent("Error: Failed to remove incomplete file: " + outputPath);
        }
    }

    curl_easy_cleanup(curl);
    return success;
}

// Загрузка нескольких файлов параллельно с ограничением по количеству потоков
void downloadMultipleFiles(const std::vector<std::string> &urls,
                           const std::string &outputDir,
                           size_t maxThreads)
{
    size_t hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0)
    {
        hardwareThreads = 2;
    }

    maxThreads = std::min(maxThreads, hardwareThreads);

    if (std::filesystem::exists(outputDir))
    {
        for (const auto &entry : std::filesystem::directory_iterator(outputDir))
        {
            std::lock_guard<std::mutex> lock(dirFileNamesMutex);
            dirFileNames.insert(entry.path().filename().string());
        }
    }

    std::vector<std::thread> threads;

    size_t activeThreads = 0;

    for (size_t i = 0; i < urls.size(); ++i)
    {
        // Создаем поток для каждого URL
        threads.emplace_back([&, i]()
                             {
            bool success = downloadFile(urls[i], outputDir);
            if (!success) {
                logEvent("Skipping URL due to failure: " + urls[i]);
            } });

        activeThreads++;

        if (activeThreads >= maxThreads)
        {
            for (auto &thread : threads)
            {
                if (thread.joinable())
                    thread.join();
            }
            threads.clear();
            activeThreads = 0;
        }
    }

    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}
