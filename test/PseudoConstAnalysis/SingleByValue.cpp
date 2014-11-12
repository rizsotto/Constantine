// RUN: %clang_verify %s

void test_1() {
    int i = 0; // expected-warning {{variable 'i' could be declared as const}}
    int const k = i;
}

int test_2(int const a) {
    int i = a; // expected-warning {{variable 'i' could be declared as const}}
    return i;
}

int test_3() {
    int i = // expected-warning {{variable 'i' could be declared as const}}
        test_2(2);
    return i;
}
