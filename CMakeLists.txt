# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(DownloaderProject VERSION 1.0 LANGUAGES CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Устанавливаем пути к исходным файлам
set(SOURCES
    main.cpp
    downloader.cpp
)

set(HEADERS
    downloader.h
)

# Добавляем исполняемый файл
add_executable(downloader ${SOURCES} ${HEADERS})

# Подключаем библиотеку libcurl
find_package(CURL REQUIRED)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(downloader ${CURL_LIBRARIES})
else()
    message(FATAL_ERROR "libcurl not found!")
endif()

# Добавляем поддержку файловой системы для старых компиляторов
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(downloader stdc++fs)
endif()

# Установка исполняемого файла в директорию bin
install(TARGETS downloader DESTINATION bin)


