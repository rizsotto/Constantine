include(GNUInstallDirs)

include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINITIONS})
add_definitions(-std=c++11)

add_library(constantine SHARED
    UsageCollector.cpp
    DeclarationCollector.cpp
    ScopeAnalysis.cpp
    PluginMain.cpp
    ModuleAnalysis.cpp
)
set_target_properties(constantine PROPERTIES
    LINKER_LANGUAGE CXX
    LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/ExportedSymbolsList"
    SOVERSION 1.0)

install(TARGETS constantine
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
