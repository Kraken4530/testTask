cmake_minimum_required(VERSION 3.10)

project(DownloaderProject VERSION 1.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Установка пути к исходным файлам
set(SOURCES
    main.cpp
    downloader.cpp
)

set(HEADERS
    downloader.h
)

# Добавление исполняемого файла
add_executable(downloader ${SOURCES} ${HEADERS})

find_package(Poco REQUIRED Net NetSSL Foundation)

target_link_libraries(downloader PRIVATE Poco::Net Poco::NetSSL Poco::Foundation)
