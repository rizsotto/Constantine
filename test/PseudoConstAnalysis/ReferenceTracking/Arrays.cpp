// RUN: %verify_const %s

void test_1() {
    int i[] = { 0, 1, 2 }; // expected-warning {{variable 'i' could be declared as const}}
    int const k = i[0];
}

void test_2() {
    int i[] = { 0, 1, 2 }; // expected-warning {{variable 'i' could be declared as const}}
    int const & k = i[0];
}

void test_3() {
    int i[] = { 0, 1, 2 }; // expected-warning {{variable 'i' could be declared as const}}
    int const * const k = i;
}

void test_4() {
    int i[] = { 0, 1, 2 };
    int & k = i[0];
    ++k;
}

void test_5() {
    int i[] = { 0, 1, 2 };
    int * const k = i;
    ++(*k);
}
