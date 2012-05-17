// RUN: %clang_cc1 %s -fsyntax-only -verify

namespace {

    int j = 9; // expected-warning {{variable could be declared as const [Medve plugin]}}

    int add(int) { return j; }

    namespace zone {
    
        int add(int);

        int add(int const k) { return k + j; }

        int k = 1; // expected-warning {{variable could be declared as const [Medve plugin]}}
    }

    int inc(int) { return zone::k; }
}
