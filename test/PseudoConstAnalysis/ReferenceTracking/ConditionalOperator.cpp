// RUN: %clang_verify %s

void do_mutating_through_reference(bool const a) {
    int i = 0;
    int j = 0;
    int & k = (a) ? i : j;
    ++k;
}

void mutated_by_value_but_reference_reported(bool const a) {
    int i = 0;
    int j = 0;
    int & k = (a) ? i : j; // expected-warning {{variable 'k' could be declared as const}}
    ++i;
    ++j;
}

void do_mutating_through_pointer(bool const a) {
    int i = 0;
    int j = 0;
    int * k = (a) ? &i : &j;
    ++(*k);
}

void mutated_by_value_but_pointer_reported(bool const a) {
    int i = 0;
    int j = 0;
    int * k = (a) ? &i : &j; // expected-warning {{variable 'k' could be declared as const}}
    ++i;
    ++j;
}
