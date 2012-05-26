// RUN: %clang_cc1 %s -fsyntax-only -verify

int inc(int k) {
    return ++k;
}

void big_test_method() {
    int k = 1; // expected-warning {{variable could be declared as const [Medve plugin]}}
    inc(k);
}

namespace {
    void test_method_in_namespace() {
        int k = 1; // expected-warning {{variable could be declared as const [Medve plugin]}}
        inc(k);
    }
}

