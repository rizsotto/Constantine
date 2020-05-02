add_library(constantine_a OBJECT
        libconstantine_a/DeclarationCollector.cpp
        libconstantine_a/ModuleAnalysis.cpp
        libconstantine_a/ScopeAnalysis.cpp
        )

target_include_directories(constantine_a PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(constantine_a PUBLIC ${CLANG_DEFINITIONS})
target_include_directories(constantine_a PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(constantine_a PUBLIC cxx_std_14)
target_compile_options(constantine_a PUBLIC -fno-exceptions -fno-rtti)
set_target_properties(constantine_a PROPERTIES
        LINKER_LANGUAGE CXX
        POSITION_INDEPENDENT_CODE ON)


add_library(constantine SHARED
        libconstantine/PluginMain.cpp
        )

target_link_libraries(constantine constantine_a)
set_target_properties(constantine PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)

include(GNUInstallDirs)
install(TARGETS constantine
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


add_library(debug SHARED
        libdebug/DebugModuleAnalysis.cpp
        libdebug/PluginMain.cpp
        )

target_link_libraries(debug constantine_a)
set_target_properties(debug PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)
