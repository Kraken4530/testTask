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

# Подключение библиотеки libcurl
find_package(CURL REQUIRED)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(downloader ${CURL_LIBRARIES})
else()
    message(FATAL_ERROR "libcurl not found!")
endif()




