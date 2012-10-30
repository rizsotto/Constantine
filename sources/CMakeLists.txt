# This file is distributed under MIT-LICENSE. See COPYING for details.

set(ourlib_target constantine)

set(ourlib_sources
    UsageCollector.cpp
    DeclarationCollector.cpp
    ScopeAnalysis.cpp
    PluginMain.cpp
    ModuleAnalysis.cpp
)

set(ourlib_symbols ${CMAKE_CURRENT_SOURCE_DIR}/ExportedSymbolsList)


add_library(${ourlib_target} SHARED ${ourlib_sources})

include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINITIONS})

set_target_properties(${ourlib_target} PROPERTIES
    LINKER_LANGUAGE CXX)
set_target_properties(${ourlib_target} PROPERTIES
    SOVERSION 1.0)
set_target_properties(${ourlib_target} PROPERTIES
    PREFIX "lib")
set_target_properties(${ourlib_target} PROPERTIES
    LINK_FLAGS "-Wl,--version-script=${ourlib_symbols}")

install(TARGETS ${ourlib_target}
    LIBRARY DESTINATION lib)
