// RUN: %clang_cc1 %s -fsyntax-only -verify

int single_argument(int j) { // expected-warning {{variable 'j' could be declared as const}}
    return (j == 0) ? 1 : 2;
}

int first_argument(int j // expected-warning {{variable 'j' could be declared as const}}
    , int k) {
    return (j == --k) ? 1 : 2;
}

int both_arguments(int j // expected-warning {{variable 'j' could be declared as const}}
    , int k) { // expected-warning {{variable 'k' could be declared as const}}
    return (j == k) ? 1 : 2;
}
