#include "downloader.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

std::vector<std::string> readUrlsFromFile(const std::string &filePath)
{
    std::vector<std::string> urls;
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open file: " << filePath << std::endl;
        return urls;
    }

    std::string line;
    while (std::getline(inputFile, line))
    {
        if (!line.empty())
        {
            urls.push_back(line);
        }
    }

    inputFile.close();
    return urls;
}

int main(int argc, char *argv[])
{
    logEvent("Program started.");

    // Проверка, что аргументов достаточно
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <file_path> <dir_path> <max_threads>" << std::endl;
        return 1; // Завершение программы с ошибкой
    }

    // Извлечение аргументов
    std::string str1 = argv[1]; // Путь к файлу
    std::vector<std::string> urls = readUrlsFromFile(str1);
    std::string str2 = argv[2]; // Директория для сохранения
    std::string outputDir = str2;
    size_t maxThreads;

    // Конвертация третьего аргумента в целое число
    try
    {
        maxThreads = std::stoi(argv[3]);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: Third argument must be an integer." << std::endl;
        return 1; // Завершение программы с ошибкой
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Error: Integer out of range." << std::endl;
        return 1; // Завершение программы с ошибкой
    }

    downloadMultipleFiles(urls, outputDir, maxThreads);

    logEvent("Program finished.");

    return 0; // Успешное завершение
}
