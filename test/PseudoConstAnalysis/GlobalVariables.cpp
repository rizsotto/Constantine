// RUN: %clang_cc1 %s -fsyntax-only -verify

namespace {

    int vary = 9; // expected-warning {{variable could be declared as const [Medve plugin]}}

    int foo(int) { return vary; }

    namespace zone {
    
        int add_constant(int);

        int add_constant(int const konst) { return konst + vary; }

        int vary = 1; // expected-warning {{variable could be declared as const [Medve plugin]}}
    }

    int bar(int) { return zone::vary; }
}
