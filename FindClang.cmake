# input:
#   LLVM_SRC_DIR    - where are the llvm sources
#   LLVM_BUILD_DIR  - where are the llvm libs
# output:
#   CLANG_FOUND
#   CLANG_INCLUDE_DIRS
#   CLANG_DEFINES

set(CLANG_SRC_DIR "${LLVM_SRC_DIR}/tools/clang")
set(CLANG_BUILD_DIR "${LLVM_BUILD_DIR}/tools/clang")

set(include_search_path ${include_search_path} "${LLVM_SRC_DIR}/include")
set(include_search_path ${include_search_path} "${CLANG_SRC_DIR}/include")
set(include_search_path ${include_search_path} "${LLVM_BUILD_DIR}/include")
set(include_search_path ${include_search_path} "${CLANG_BUILD_DIR}/include")

set(clang_cflags ${clang_cflags} -D_GNU_SOURCE)
set(clang_cflags ${clang_cflags} -D__STDC_LIMIT_MACROS)
set(clang_cflags ${clang_cflags} -D__STDC_CONSTANT_MACROS)
set(clang_cflags ${clang_cflags} -DHAVE_CLANG_CONFIG_H)
set(clang_cflags ${clang_cflags} -fno-rtti)

set(lib_search_dir "${LLVM_BUILD_DIR}/lib")

set(CLANG_FOUND 1)
set(CLANG_INCLUDE_DIRS  ${include_search_path})
set(CLANG_DEFINES       ${clang_cflags})

