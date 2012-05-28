Medve
=====

Medve is a toy project to learn how to write clang plugin.

Implemented functionalities
---------------------------

* Currently implemented const analysis for basic types.

Prerequisites
-------------

You need a copy of the clang 3.1 compiler. Or a newer from SVN.
Description for that you can find from [clang getting started][1].

r157581 (from trunk) is known to be working. (Mon, 28 May 2012)

How to build
------------

* Make sure that `llvm-config`, `llvm-lit` and `clang` binaries are
in your `PATH`. Or pass `LLVM_PATH` environment variable at cmake step.
* `mkdir build && cd build`
* `cmake .. && make`
* To run test suite `make check`

[1]: http://clang.llvm.org/get_started.html     "clang getting started"

