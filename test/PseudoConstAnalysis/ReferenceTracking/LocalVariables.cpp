// RUN: %clang_cc1 %s -fsyntax-only -verify

void do_mutating_through_reference() {
    int i = 0;
    int & k = i;
    ++k;
}

void mutated_by_value_but_reference_reported() {
    int i = 0;
    int & k = i; // expected-warning {{variable 'k' could be declared as const}}
    ++i;
}

void do_mutating_through_pointer() {
    int i = 0;
    int * k = &i;
    ++(*k);
}

void mutated_by_value_but_pointer_reported() {
    int i = 0;
    int * k = &i; // expected-warning {{variable 'k' could be declared as const}}
    ++i;
}
