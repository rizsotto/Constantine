Medve
=====

Medve is a toy project to learn how to write clang plugin.

Implemented functionalities
---------------------------

* Implement pseudo const analysis.

Prerequisites
-------------

Clang 3.1 or a newer from SVN. Description for that you can find
from [clang getting started][1].

r164964 (from trunk) is known to be working. (Mon,  1 Oct 2012)

How to build
------------

* Make sure that `llvm-config`, `llvm-lit` and `clang` binaries are
in your `PATH`. Or pass `LLVM_PATH` environment variable at cmake step.
* `mkdir build && cd build`
* `cmake .. && make`
* To run test suite `make check`

[1]: http://clang.llvm.org/get_started.html     "clang getting started"

