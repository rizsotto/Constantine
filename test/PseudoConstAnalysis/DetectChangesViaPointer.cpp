// RUN: %clang_cc1 %s -fsyntax-only -verify

void pointer_declaration_test() {
    int k = 0;

    int * p1 = &k; // expected-warning {{variable could be declared as const}}
    int * const p2 = &k;
    int const * p3 = &k; // expected-warning {{variable could be declared as const}}
}

void pointer_pointer_declaration_test() {
    int k = 0;

    int * const p1 = &k;
    int * const * pp1 = &p1; // expected-warning {{variable could be declared as const}}
}

int use_argument_via_const_pointer(int const * const k) {
    return (*k == 9) ? 1 : 2;
}

void const_pointer_declaration_test() {
    int k = 9; // would be nice
    int const * const p = &k;
}

void const_pointer_passing_test() {
    int k = 9; // would be nice
    use_argument_via_const_pointer(&k);
}

void pointer_value_test() {
    int k = 0;

    int * const p1 = &k; // would be nice

    int * const p2 = &k;
    *p2 += 3;
}
