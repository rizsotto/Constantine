# Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

# output:
#   CLANG_FOUND
#   CLANG_INCLUDE_DIRS
#   CLANG_DEFINES

execute_process(COMMAND llvm-config --src-root OUTPUT_VARIABLE llvm_src_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --obj-root OUTPUT_VARIABLE llvm_obj_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --cppflags OUTPUT_VARIABLE clang_cflags OUTPUT_STRIP_TRAILING_WHITESPACE)

set(clang_src_dir "${llvm_src_dir}/tools/clang")
set(clang_obj_dir "${llvm_obj_dir}/tools/clang")

set(include_search_path ${include_search_path} "${clang_src_dir}/include")
set(include_search_path ${include_search_path} "${clang_obj_dir}/include")
set(clang_cflags ${clang_cflags} -fno-rtti)

set(CLANG_FOUND 1)
set(CLANG_INCLUDE_DIRS  ${include_search_path})
set(CLANG_DEFINES       ${clang_cflags})

