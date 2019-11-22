add_library(constantine SHARED
        PluginMain.cpp)

target_link_libraries(constantine constantine_a)

target_include_directories(constantine PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(constantine PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(constantine PUBLIC cxx_std_14)
target_compile_options(constantine PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(constantine PROPERTIES
        LINKER_LANGUAGE CXX
        SOVERSION 1.0)

include(GNUInstallDirs)
install(TARGETS constantine
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
