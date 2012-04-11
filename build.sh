#!/bin/env bash
set -o nounset
set -o errexit

script_dir=$(readlink -m $(dirname "${BASH_SOURCE[0]}"))
build_dir=$(pwd)
llvm_source_dir="$script_dir/../llvm.from.svn"
llvm_build_dir="$llvm_source_dir/build"

cmake $script_dir -DLLVM_SRC_DIR=$llvm_source_dir -DLLVM_BUILD_DIR=$llvm_build_dir

make

clangbin=$llvm_build_dir/bin/clang++
stdincs=$($clangbin -v -c -x c++ /dev/null 2>&1 | grep internal | awk '{ for (i = 1; i<=NF; i++) { if (match($i, "-internal-")) { k = ++i; printf("-I%s ", $k) } } }')

$llvm_build_dir/bin/clang++ -cc1 $stdincs -load $build_dir/sources/libFindConstCandidates.so -plugin find-const-candidate $script_dir/test/first.cpp
