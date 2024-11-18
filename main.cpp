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
        // Добавляем строку в вектор, если она не пуста
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

    // Проверяем, что аргументов достаточно
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <string1> <string2> <integer>" << std::endl;
        return 1; // Завершаем программу с кодом ошибки
    }

    // Извлекаем аргументы
    std::string str1 = argv[1]; // Первая строка
    std::vector<std::string> urls = readUrlsFromFile(str1);
    std::string str2 = argv[2]; // Вторая строка
    std::string outputDir = str2;
    size_t maxThreads;

    // Конвертируем третий аргумент в целое число
    try
    {
        maxThreads = std::stoi(argv[3]); // Преобразование строки в int
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: Third argument must be an integer." << std::endl;
        return 1; // Завершаем программу с кодом ошибки
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Error: Integer out of range." << std::endl;
        return 1; // Завершаем программу с кодом ошибки
    }

    downloadMultipleFiles(urls, outputDir, maxThreads);

    logEvent("Program finished.");

    return 0; // Успешное завершение
}
