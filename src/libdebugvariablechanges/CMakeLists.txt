add_library(debugvariablechanges SHARED
        PluginMain.cpp)

target_link_libraries(debugvariablechanges constantine_a)

target_include_directories(debugvariablechanges PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(debugvariablechanges PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(debugvariablechanges PUBLIC cxx_std_14)
target_compile_options(debugvariablechanges PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(debugvariablechanges PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)
