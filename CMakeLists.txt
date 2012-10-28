# This file is distributed under MIT-LICENSE. See COPYING for details.

cmake_minimum_required(VERSION 2.6)
project(constantine)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(Clang REQUIRED)

add_subdirectory(sources)
add_subdirectory(test)
