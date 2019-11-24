add_library(debug SHARED
        DebugModuleAnalysis.hpp
        DebugModuleAnalysis.cpp
        PluginMain.cpp)

target_link_libraries(debug constantine_a)

target_include_directories(debug PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(debug PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(debug PUBLIC cxx_std_14)
target_compile_options(debug PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(debug PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)
