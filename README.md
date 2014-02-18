Constantine
===========

Constantine is a toy project to learn how to write Clang plugin.

Implements pseudo const analysis. Generates warnings about variables,
which were declared without const qualifier.


How to build
------------

Constantine was tested on Linux only.
For Unbuntu/Debian build, you can take a look at the `.travis.yml` file.

### Prerequisites

1. **C++ compiler** to compile the sources.
2. **cmake** to configure the build process.
3. **make** to run the build. Makefiles generated by `cmake`.

4. Install **LLVM/Clang**. Either you do install from sources or package
   for your distribution, the Clang version shall match with Constantine version.
   Make sure that `llvm-config` and `clang` executables are in the `PATH`
   environment.

5. Install **Lit**. This is optional, do only if you want to run the tests!
   Lit is the [LLVM Integrated Tester][LIT]. If you installed Clang from
   sources, you shall have it automaticaly. If your package manager does
   not provide it, you can simply install from [PyPI][PyPI] because it is
   written in python.

6. Install **Boost C++ Libraries**, version 1.46.1 or greater. Constantine
   relies on header-only libraries.

   [LIT]: http://llvm.org/docs/CommandGuide/lit.html
   [PyPI]: https://pypi.python.org/pypi/lit

### Build Constantine

It could be the best to build it in a separate build directory.

    cmake $CONSTANTINE_SOURCE_DIR
    make all
    make install  # to install
    make check    # to run tests
    make package  # to create tgz, rpm, deb packages

You can configure the build process with passing arguments to cmake.


How to use
----------

To run the plugin against your sources you need to tune the build
script of your project. Sure you need to replace the compiler to
Clang. To hook the plugin into the Clang driver, you need to pass
extra flags which are Clang specific, therefore those are 'escaped'
like this:

    CC="clang"
    CXX="clang++"
    CXX_FLAGS+=" -Xclang -load -Xclang $CONSTANTINE_LIB_PATH/libconstantine.so"
    CXX_FLAGS+=" -Xclang -add-plugin -Xclang constantine"


Problem reports
---------------

If you find a bug in this documentation or elsewhere in the program or would
like to propose an improvement, please use the project's [github issue
tracker][ISSUES]. Please describing the bug and where you found it. If you
have a suggestion how to fix it, include that as well. Patches are also
welcome.

  [ISSUES]: https://github.com/rizsotto/Constantine/issues
