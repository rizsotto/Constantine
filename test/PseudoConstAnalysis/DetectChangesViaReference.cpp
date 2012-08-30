// RUN: %clang_cc1 %s -fsyntax-only -verify

void reference_declaration_test() {
    int k = 0; // would be nice

    int & r = k; // expected-warning {{variable 'r' could be declared as const}}

    int const j = r + k;
}

void const_reference_declaration_test() {
    int k = 9; // expected-warning {{variable 'k' could be declared as const}}
    int const & r = k;

    int const j = r + k;
}

int use_argument_via_const_reference(int const & k) {
    return (k == 9) ? 1 : 2;
}

void const_reference_passing_test() {
    int k = 9; // expected-warning {{variable 'k' could be declared as const}}
    use_argument_via_const_reference(k);
}

void reference_value_test() {
    int k = 0;

    int & r = k;
    r += 3;
}
