set(ourlib_target FindConstCandidates)

set(ourlib_sources
    PrintFunctionNames.cpp
)

set(ourlib_symbols PrintFunctionNames.exports)


add_library(${ourlib_target} SHARED ${ourlib_sources})

include_directories(${CLANG_INCLUDE_DIRS})
target_link_libraries(${ourlib_target} ${CLANG_LIBRARIES})
add_definitions(${CLANG_DEFINES})

set_target_properties(${ourlib_target} PROPERTIES
    LINKER_LANGUAGE CXX)
set_target_properties(${ourlib_target} PROPERTIES
    PREFIX "lib")
set_target_properties(${ourlib_target} PROPERTIES
    LINK_FlAGS "-exported_symbols_list ${ourlib_symbols}")
