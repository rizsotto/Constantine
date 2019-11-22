add_library(debugfunctiondeclarations SHARED
        PluginMain.cpp)

target_link_libraries(debugfunctiondeclarations constantine_a)

target_include_directories(debugfunctiondeclarations PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(debugfunctiondeclarations PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(debugfunctiondeclarations PUBLIC cxx_std_14)
target_compile_options(debugfunctiondeclarations PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(debugfunctiondeclarations PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)
