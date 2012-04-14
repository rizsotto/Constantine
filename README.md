Medve
=====

Medve is a toy project to learn how to write clang plugin.

Implemented functionalities
---------------------------

* Currently implemented explicit cast lookup.

Prerequisites
-------------

You need a copy of the upcoming clang 3.1 compiler. Since it is not released
yet, you need to check out from SVN. Description for that you can find from
[clang getting started][1].

r154753 is known to be working. (Sat, 14 Apr 2012)

How to build
------------

* Make sure that `llvm-config` is in your `PATH`
* `mkdir build && cd build`
* `cmake .. && make`

[1]: http://clang.llvm.org/get_started.html      Clang getting started

