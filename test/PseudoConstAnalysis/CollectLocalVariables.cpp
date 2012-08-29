// RUN: %clang_cc1 %s -fsyntax-only -verify

int test_method() {
    int k = 1; // expected-warning {{variable could be declared as const}}
    return (k == 9) ? 1 : 2;
}

namespace {
    int test_method_in_namespace() {
        int k = 1; // expected-warning {{variable could be declared as const [Medve plugin]}}
        return (k == 9) ? 1 : 2;
    }
}

