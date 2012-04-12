# Copyright 2011 by Laszlo Nagy [see file LICENSE.TXT]

cmake_minimum_required(VERSION 2.6)
project(ConSug)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
find_package(Clang REQUIRED)

add_subdirectory(sources)
