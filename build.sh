#!/bin/env bash

script_dir=$(readlink -m $(dirname "${BASH_SOURCE[0]}"))
build_dir=$(pwd)
llvm_source_dir="$script_dir/../llvm.from.svn"
llvm_build_dir="$llvm_source_dir/build"

cmake $script_dir -DLLVM_SRC_DIR=$llvm_source_dir -DLLVM_BUILD_DIR=$llvm_build_dir

make VERBOSE=1

$llvm_build_dir/bin/clang++ -cc1 -load $build_dir/sources/libFindConstCandidates.so -plugin print-fns $script_dir/test/first.cpp
