# Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

set(ourlib_target Medve)

set(ourlib_sources
    PluginMain.cpp
)

set(ourlib_symbols ExportedSymbolsList)


add_library(${ourlib_target} SHARED ${ourlib_sources})

include_directories(${CLANG_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINES})

set_target_properties(${ourlib_target} PROPERTIES
    LINKER_LANGUAGE CXX)
set_target_properties(${ourlib_target} PROPERTIES
    PREFIX "lib")
set_target_properties(${ourlib_target} PROPERTIES
    LINK_FlAGS "-exported_symbols_list ${ourlib_symbols}")
