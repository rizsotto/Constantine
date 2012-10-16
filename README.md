Medve
=====

Medve is a toy project to learn how to write clang plugin.

Implemented functionalities
---------------------------

* Implement pseudo const analysis.

Prerequisites
-------------

Clang from SVN. Description for that you can find from [clang getting started][1].

r165486 (from trunk) is known to be working. (Tue,  9 Oct 2012)

How to build
------------

* Make sure that `llvm-config`, `llvm-lit` and `clang` binaries are
in your `PATH`. Or pass `LLVM_PATH` environment variable at cmake step.
* `mkdir build && cd build`
* `cmake .. && make`
* To run test suite `make check`

How to use
----------

Set your compiler to `clang` and pass some plugin related flags.

```shell
$ export CC="$LLVM_HOME/clang"
$ export CXX="$LLVM_HOME/clang++"
$ export CXXFLAGS="-Xclang -load -Xclang $MEDVE_PATH/libMedve.so -Xclang -add-plugin -Xclang medve"
```

[1]: http://clang.llvm.org/get_started.html     "clang getting started"

