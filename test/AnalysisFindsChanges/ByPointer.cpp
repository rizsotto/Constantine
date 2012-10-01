// RUN: %clang_cc1 %change %s -fsyntax-only -verify

void unary_operators() {
    int i = 0;
    int * iptr = &i;

    ++iptr; // expected-note {{variable 'iptr' with type 'int *' was changed}}
    --iptr; // expected-note {{variable 'iptr' with type 'int *' was changed}}
    iptr++; // expected-note {{variable 'iptr' with type 'int *' was changed}}
    iptr--; // expected-note {{variable 'iptr' with type 'int *' was changed}}

    ++(*iptr); // expected-note {{variable 'iptr' with type 'int' was changed}}
    --(*iptr); // expected-note {{variable 'iptr' with type 'int' was changed}}
    (*iptr)++; // expected-note {{variable 'iptr' with type 'int' was changed}}
    (*iptr)--; // expected-note {{variable 'iptr' with type 'int' was changed}}
}

void binary_operators() {
    int i = 0;
    int * iptr = &i;

    *iptr = 0; // expected-note {{variable 'iptr' with type 'int' was changed}}
    iptr = 0; // expected-note {{variable 'iptr' with type 'int *' was changed}}
    *(&i) = 1; // expected-note {{variable 'i' with type 'int' was changed}}

    *iptr *= 1; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr /= 1; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr %= 2; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr += 2; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr -= 2; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr <<= 1; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr >>= 1; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr |= 2; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr ^= 2; // expected-note {{variable 'iptr' with type 'int' was changed}}
    *iptr &= 1; // expected-note {{variable 'iptr' with type 'int' was changed}}
}
