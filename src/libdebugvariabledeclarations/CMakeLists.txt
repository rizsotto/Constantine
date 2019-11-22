add_library(debugvariabledeclarations SHARED
        PluginMain.cpp)

target_link_libraries(debugvariabledeclarations constantine_a)

target_include_directories(debugvariabledeclarations PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(debugvariabledeclarations PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(debugvariabledeclarations PUBLIC cxx_std_14)
target_compile_options(debugvariabledeclarations PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(debugvariabledeclarations PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)
