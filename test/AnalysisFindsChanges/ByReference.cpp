// RUN: %clang_cc1 %change %s -fsyntax-only -verify

void unary_operators() {
    int i = 0;
    int & iref = i;

    ++iref; // expected-note {{variable 'iref' with type 'int' was changed}}
    --iref; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref++; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref--; // expected-note {{variable 'iref' with type 'int' was changed}}
}

void binary_operators() {
    int i = 0;
    int & iref = i;

    iref = 1; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref *= 1; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref /= 1; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref %= 2; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref += 2; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref -= 2; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref <<= 1; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref >>= 1; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref |= 2; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref ^= 2; // expected-note {{variable 'iref' with type 'int' was changed}}
    iref &= 1; // expected-note {{variable 'iref' with type 'int' was changed}}
}
