# This file is distributed under MIT-LICENSE. See COPYING for details.

cmake_minimum_required(VERSION 2.8)
project(constantine)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Boost REQUIRED)
find_package(Clang REQUIRED)

add_subdirectory(sources)
add_subdirectory(test)
