cmake_minimum_required(VERSION 2.6)
project(ConSug)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(Threads REQUIRED)
find_package(Clang REQUIRED)

add_subdirectory(sources)
