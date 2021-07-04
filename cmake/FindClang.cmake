# output:
#   CLANG_FOUND
#   CLANG_INCLUDE_DIRS
#   CLANG_DEFINITIONS
#   CLANG_EXECUTABLE

function(set_clang_definitions config_cmd)
  execute_process(
    COMMAND ${config_cmd} --cppflags
    OUTPUT_VARIABLE llvm_cppflags
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCHALL "(-D[^ ]*)" dflags ${llvm_cppflags})
  string(REGEX MATCHALL "(-U[^ ]*)" uflags ${llvm_cppflags})
  list(APPEND cxxflags ${dflags})
  list(APPEND cxxflags ${uflags})

  set(CLANG_DEFINITIONS ${cxxflags} PARENT_SCOPE)
endfunction()

function(is_clang_installed config_cmd)
  execute_process(
    COMMAND ${config_cmd} --includedir
    OUTPUT_VARIABLE include_dirs
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND ${config_cmd} --src-root
    OUTPUT_VARIABLE llvm_src_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(FIND ${include_dirs} ${llvm_src_dir} result)

  set(CLANG_INSTALLED ${result} PARENT_SCOPE)
endfunction()

function(set_clang_include_dirs config_cmd)
  is_clang_installed(${config_cmd})
  if(CLANG_INSTALLED)
    execute_process(
      COMMAND ${config_cmd} --includedir
      OUTPUT_VARIABLE include_dirs
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    execute_process(
      COMMAND ${config_cmd} --src-root
      OUTPUT_VARIABLE llvm_src_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
      COMMAND ${config_cmd} --obj-root
      OUTPUT_VARIABLE llvm_obj_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND include_dirs "${llvm_src_dir}/include")
    list(APPEND include_dirs "${llvm_obj_dir}/include")
    list(APPEND include_dirs "${llvm_src_dir}/tools/clang/include")
    list(APPEND include_dirs "${llvm_obj_dir}/tools/clang/include")
  endif()

  set(CLANG_INCLUDE_DIRS ${include_dirs} PARENT_SCOPE)
endfunction()


find_program(LLVM_CONFIG
  NAMES llvm-config-12 llvm-config-11 llvm-config-10 llvm-config-9 llvm-config-8 llvm-config llvm-config-64
  PATHS ENV LLVM_PATH)
if(LLVM_CONFIG)
  message(STATUS "llvm-config found : ${LLVM_CONFIG}")
else()
  message(FATAL_ERROR "Can't found program: llvm-config")
endif()

find_program(CLANG_EXECUTABLE
  NAMES clang-12 clang-11 clang-10 clang-9 clang-8 clang
  PATHS ENV LLVM_PATH)
if(CLANG_EXECUTABLE)
  message(STATUS "clang found : ${CLANG_EXECUTABLE}")
else()
  message(FATAL_ERROR "Can't found program: clang")
endif()

set_clang_definitions(${LLVM_CONFIG})
set_clang_include_dirs(${LLVM_CONFIG})

message(STATUS "llvm-config filtered cpp flags : ${CLANG_DEFINITIONS}")
message(STATUS "llvm-config filtered include dirs : ${CLANG_INCLUDE_DIRS}")

set(CLANG_FOUND 1)
