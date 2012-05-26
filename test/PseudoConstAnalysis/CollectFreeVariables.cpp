// RUN: %clang_cc1 %s -fsyntax-only -verify

namespace {

    int konst = 9; // expected-warning {{variable could be declared as const [Medve plugin]}}
    int vary = 8;

    int foo(int) { return konst; }

    namespace zone {
    
        int add_constant(int);

        int add_constant(int const konst) { return konst + konst; }

        int konzt = 1; // expected-warning {{variable could be declared as const [Medve plugin]}}
    }

    int bar(int) { return zone::konzt; }

    void change() { vary += 9; }
}
