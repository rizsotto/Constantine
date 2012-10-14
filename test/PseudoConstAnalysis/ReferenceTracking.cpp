// RUN: %clang_cc1 %s -fsyntax-only -verify

void test_1() {
    int i = 0;
    int & k = i;
    ++k;
}

void test_2() {
    int i = 0;
    int & k = i; // expected-warning {{variable 'k' could be declared as const}}
    ++i;
}

void test_3() {
    int i = 0;
    int * k = &i;
    ++(*k);
}

void test_4() {
    int i = 0;
    int * k = &i; // expected-warning {{variable 'k' could be declared as const}}
    ++i;
}
