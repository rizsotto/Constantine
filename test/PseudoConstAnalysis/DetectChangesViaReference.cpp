// RUN: %clang_cc1 %s -fsyntax-only -verify

void reference_declaration_test() {
    int k = 0; // would be nice

    int & r = k; // expected-warning {{variable could be declared as const [Medve plugin]}}

    int const j = r + k;
}

void const_reference_declaration_test() {
    int k = 9; // expected-warning {{variable could be declared as const [Medve plugin]}}
    int const & r = k;

    int const j = r + k;
}

void dont_change(int const &) { }

void const_reference_passing_test() {
    int k = 9; // expected-warning {{variable could be declared as const [Medve plugin]}}
    dont_change(k);
}

void reference_value_test() {
    int k = 0;

    int & r = k;
    r += 3;
}
