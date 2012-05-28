# Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

# output:
#   CLANG_FOUND
#   CLANG_INCLUDE_DIRS
#   CLANG_DEFINES

find_program(LLVM_CONFIG llvm-config
    PATH ENV LLVM_PATH)
find_program(LLVM_LIT llvm-lit
    PATH ENV LLVM_PATH)
find_program(CLANG_BIN clang
    PATH ENV LLVM_PATH)

if (LLVM_CONFIG AND LLVM_LIT AND CLANG_BIN)
  execute_process(COMMAND ${LLVM_CONFIG} --version OUTPUT_VARIABLE llvm_version OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${LLVM_CONFIG} --src-root OUTPUT_VARIABLE llvm_src_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${LLVM_CONFIG} --obj-root OUTPUT_VARIABLE llvm_obj_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${LLVM_CONFIG} --cppflags OUTPUT_VARIABLE clang_cflags OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(clang_src_dir "${llvm_src_dir}/tools/clang")
  set(clang_obj_dir "${llvm_obj_dir}/tools/clang")

  set(include_search_path ${include_search_path} "${clang_src_dir}/include")
  set(include_search_path ${include_search_path} "${clang_obj_dir}/include")
  set(clang_cflags ${clang_cflags} -fno-rtti)

  set(CLANG_FOUND 1)
  set(CLANG_INCLUDE_DIRS  ${include_search_path})
  set(CLANG_DEFINES       ${clang_cflags})

  message(STATUS "llvm-config found : ${LLVM_CONFIG}")
  message(STATUS "llvm-config --version : ${llvm_version}")
  message(STATUS "llvm-lit found : ${LLVM_LIT}")
else()
  message(FATAL_ERROR "llvm programs not found. LLVM_PATH environment variable.")
endif()
