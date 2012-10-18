// RUN: %clang_cc1 %s -fsyntax-only -verify

void test_1() {
    int i[] = { 0, 1, 2 };
    i[0] = 1;
}

void test_2() {
    int i[] = { 0, 1, 2 };
    int const k = 0;
    i[k] = 1;
}

void test_3() {
    int i[] = { 0, 1, 2 }; // expected-warning {{variable 'i' could be declared as const}}
    int const k = i[0];
}
