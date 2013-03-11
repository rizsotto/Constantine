Constantine
===========

Constantine is a toy project to learn how to write Clang plugin.

Implements pseudo const analysis. Generates warnings about variables,
which were declared without const qualifier.

Prerequisites
-------------

To build you need [Clang][1] version 3.2 and [boost][2] library installed.
To run tests you need to have [lit][3] installed on your system.

How to build
------------

For Unbuntu/Debian build, you can take a look at the `.travis.yml` file.

* Make sure that `llvm-config` and `clang` binaries are in your `PATH`.
* It is better to build it in a separate build directory.
`mkdir build && cd build`
* The configure step made by cmake: `cmake ..`
You can pass `-DCMAKE_INSTALL_PREFIX=<path>` to override the default
`/usr/local`. For more cmake control, read about the [related variables][4].
* To compile and run test suite: `make check`
* To install: `make install` You can specify `DESTDIR` environment to prefix
the `CMAKE_INSTALL_PREFIX`.
* To create packages: `make package`.

How to use
----------

Set your compiler to `clang` and pass some plugin related flags.

```shell
$ export CC="clang"
$ export CXX="clang++"
$ export CXXFLAGS="-Xclang -load -Xclang $TARGET_DIR/libconstantine.so -Xclang -add-plugin -Xclang constantine"
```

Konwn issues
------------

Issues are tracked on [the project github page][5].


[1]: http://clang.llvm.org/ "Clang: a C language family frontend for LLVM"
[2]: http://www.boost.org/ "boost homepage"
[3]: https://pypi.python.org/pypi/lit "lit: a software testing tool"
[4]: http://www.cmake.org/Wiki/CMake_Useful_Variables "cmake useful variables"
[5]: https://github.com/rizsotto/Constantine/issues "Constantine issues"
