Medve
=====

Medve is a toy project to learn how to write clang plugin.

Implemented functionalities
---------------------------

* Currently implemented const analysis for basic types.

Prerequisites
-------------

You need a copy of the upcoming clang 3.1 compiler. Since it is not released
yet, you need to check out from SVN. Description for that you can find from
[clang getting started][1].

r156985 is known to be working. (Fri, 19 May 2012)

How to build
------------

* Make sure that `llvm-config` and `clang` binaries are in your `PATH`
* `mkdir build && cd build`
* `cmake .. && make`
* To run test suite `make check`

[1]: http://clang.llvm.org/get_started.html     "clang getting started"

