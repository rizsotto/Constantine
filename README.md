Constantine
===========

Constantine is a toy project to learn how to write clang plugin.

Implements pseudo const analysis. Generates warnings about variables,
which were declared without const qualifier.

Prerequisites
-------------

Clang from SVN. Description for that you can find from [clang getting started][1].

Clanf trunk, after r165486 commit is known to be working. (Tue,  9 Oct 2012)

How to build
------------

* Make sure that `llvm-config`, `llvm-lit` and `clang` binaries are
in your `PATH`. Or pass `LLVM_PATH` environment variable at cmake step.
* It is better to build it in a separate build directory.
`mkdir build && cd build`
* The configure step made by cmake: `cmake ..`
You can pass `-DCMAKE_INSTALL_PREFIX=<path>` to override the default
`/usr/local`. For more cmake control, read about the [related variables][2].
* To compile and run test suite: `make check`
* To install: `make install` You can specify `DESTDIR` environment to prefix
the `CMAKE_INSTALL_PREFIX`.

How to use
----------

Set your compiler to `clang` and pass some plugin related flags.

```shell
$ export CC="$LLVM_HOME/clang"
$ export CXX="$LLVM_HOME/clang++"
$ export CXXFLAGS="-Xclang -load -Xclang $MEDVE_PATH/libconstantine.so -Xclang -add-plugin -Xclang constantine"
```

[1]: http://clang.llvm.org/get_started.html     "clang getting started"
[2]: http://www.cmake.org/Wiki/CMake_Useful_Variables "cmake useful variables"
