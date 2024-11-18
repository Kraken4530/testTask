cmake_minimum_required(VERSION 3.5)

project(FileSaver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-DCURL_STATICLIB)
    link_directories("${CMAKE_SOURCE_DIR}/thirdparty/openssl/" "${CMAKE_SOURCE_DIR}/thirdparty/"curl_win32)
else()
    link_directories("${CMAKE_SOURCE_DIR}/thirdparty/curl2")
endif()

file(GLOB SOURCES
     main.cpp
     downloader.cpp
     downloader.h
)

add_executable(FileSaver ${SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} openssl curl)
else()
    target_link_libraries(${PROJECT_NAME} curl)
endif()