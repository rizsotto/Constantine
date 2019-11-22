add_library(debugvariableusages SHARED
        PluginMain.cpp)

target_link_libraries(debugvariableusages constantine_a)

target_include_directories(debugvariableusages PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(debugvariableusages PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(debugvariableusages PUBLIC cxx_std_14)
target_compile_options(debugvariableusages PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(debugvariableusages PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)
