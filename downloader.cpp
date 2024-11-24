#include "downloader.h"

std::mutex consoleMutex;
std::mutex fileMutex;

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

std::string replaceInvalidFilenameChars(const std::string &filename)
{
    std::regex invalidChars("[^a-zA-Z0-9._-]");
    return std::regex_replace(filename, invalidChars, "_");
}

std::string extractFilenameFromUrl(const std::string &url)
{
    size_t lastSlash = url.find_last_of('/');
    std::string filename = (lastSlash != std::string::npos) ? url.substr(lastSlash + 1) : "downloaded_file";

    return replaceInvalidFilenameChars(filename);
}

std::string extractFilename(const std::string &contentDisposition)
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
            if (end == std::string::npos)
            {
                end = contentDisposition.length();
            }
            return contentDisposition.substr(pos, end - pos);
        }
    }
    return "";
}

// Генерация уникального имени файла с добавлением номера (если требуется)
std::string generateUniqueFilename(const std::string &outputDir, const std::string &filename)
{
    std::string uniqueFilename = filename;
    std::filesystem::path outputPath(outputDir);
    int counter = 1;

    while (std::filesystem::exists(outputPath / uniqueFilename))
    {
        size_t dotPos = filename.find_last_of('.');
        if (dotPos == std::string::npos)
        {
            uniqueFilename = filename + "(" + std::to_string(counter) + ")";
        }
        else
        {
            uniqueFilename = filename.substr(0, dotPos) + "(" + std::to_string(counter) + ")" +
                             filename.substr(dotPos);
        }
        ++counter;
    }

    return (outputPath / uniqueFilename).string();
}

// Загрузка файла по указанному URL в указанную директорию
bool downloadFile(const std::string &url, const std::string &outputDir)
{
    try
    {
        Poco::URI uri(url);
        std::string host = uri.getHost();
        std::string target = uri.getPathAndQuery(); // Включает путь и параметры запроса
        int port = uri.getPort() != 0 ? uri.getPort() : (uri.getScheme() == "https" ? 443 : 80);

        // Создание сессии
        std::unique_ptr<Poco::Net::HTTPClientSession> session;
        if (uri.getScheme() == "https")
        {
            // Инициализация SSL
            Poco::Net::initializeSSL();
            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pCertHandler =
                new Poco::Net::AcceptCertificateHandler(false);
            Poco::Net::SSLManager::instance().initializeClient(nullptr, pCertHandler, nullptr);
            session = std::make_unique<Poco::Net::HTTPSClientSession>(host, port);
            Poco::Net::uninitializeSSL();
        }
        else
        {
            session = std::make_unique<Poco::Net::HTTPClientSession>(host, port);
        }

        logEvent("Start downloading: " + url);

        // Отправка запроса
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, target);

        session->sendRequest(request);

        // Чтение ответа
        Poco::Net::HTTPResponse response;
        std::istream &responseStream = session->receiveResponse(response);

        if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
        {
            logEvent("Warning: Server responded with status " +
                     std::to_string(response.getStatus()) + " " +
                     response.getReason() + " for URL: " + url);
            return false;
        }

        std::string filename = "file";
        if (response.has("Content-Disposition"))
        {
            const std::string contentDisposition = response.get("Content-Disposition");
            std::string extractedFilename = extractFilename(contentDisposition);
            if (!extractedFilename.empty())
            {
                filename = replaceInvalidFilenameChars(extractedFilename);
            }
        }
        else
        {
            filename = extractFilenameFromUrl(url);
        }

        {
            std::lock_guard<std::mutex> lock(fileMutex);
            std::string outputFilename = generateUniqueFilename(outputDir, filename);
            std::ofstream file(outputFilename, std::ios::binary);
            if (file.is_open())
            {
                Poco::StreamCopier::copyStream(responseStream, file);
                file.close();
            }
            else
            {
                logEvent("Error: Unable to open file for writing: " + outputFilename);
                return false;
            }
            logEvent("File downloaded successfully to: " + outputFilename);
        }
        return true;
    }
    catch (const Poco::Exception &ex)
    {
        logEvent("Error: Poco exception occurred while downloading file from URL: " + url +
                 " with message: " + ex.displayText());
        return false;
    }
    catch (const std::exception &ex)
    {
        logEvent("Error: Standard exception occurred while downloading file from URL: " + url +
                 " with message: " + std::string(ex.what()));
        return false;
    }
    catch (...)
    {
        logEvent("Error: Unknown exception occurred while downloading file from URL: " + url);
        return false;
    }
}

// Загрузка нескольких файлов параллельно с ограничением по количеству потоков
void downloadMultipleFiles(const std::vector<std::string> &urls,
                           const std::string &outputDir,
                           size_t maxThreads)
{
    try
    {
        std::filesystem::create_directories(std::filesystem::path(outputDir));
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        logEvent("Error: Unable to create directories for path: " + outputDir + " with error: " + e.what());
        return;
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
