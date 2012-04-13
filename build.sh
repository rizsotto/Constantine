#!/bin/env bash
set -o nounset
set -o errexit

script_dir=$(readlink -m $(dirname "${BASH_SOURCE[0]}"))
build_dir=$(pwd)
llvm_source_dir="$script_dir/../llvm.from.svn"
llvm_build_dir="$llvm_source_dir/build"

cmake $script_dir -DLLVM_SRC_DIR=$llvm_source_dir -DLLVM_BUILD_DIR=$llvm_build_dir

make

$script_dir/wrapper -c $script_dir/test/first.cpp
