add_library(constantine SHARED
        DeclarationCollector.cpp
        ScopeAnalysis.cpp
        PluginMain.cpp
        ModuleAnalysis.cpp
)

target_include_directories(constantine PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(constantine PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(constantine PUBLIC cxx_std_14)
target_compile_options(constantine PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(constantine PROPERTIES
    LINKER_LANGUAGE CXX
    LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/ExportedSymbolsList"
    SOVERSION 1.0)

include(GNUInstallDirs)
install(TARGETS constantine
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
