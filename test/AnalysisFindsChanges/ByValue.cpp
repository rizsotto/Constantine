// RUN: %clang_verify %change %s

void unary_operators() {
    int i = 0;

    ++i; // expected-note {{variable 'i' with type 'int' was changed}}
    --i; // expected-note {{variable 'i' with type 'int' was changed}}
    i++; // expected-note {{variable 'i' with type 'int' was changed}}
    i--; // expected-note {{variable 'i' with type 'int' was changed}}
}

void binary_operators() {
    int i = 0;

    i = 1; // expected-note {{variable 'i' with type 'int' was changed}}
    i *= 1; // expected-note {{variable 'i' with type 'int' was changed}}
    i /= 1; // expected-note {{variable 'i' with type 'int' was changed}}
    i %= 2; // expected-note {{variable 'i' with type 'int' was changed}}
    i += 2; // expected-note {{variable 'i' with type 'int' was changed}}
    i -= 2; // expected-note {{variable 'i' with type 'int' was changed}}
    i <<= 1; // expected-note {{variable 'i' with type 'int' was changed}}
    i >>= 1; // expected-note {{variable 'i' with type 'int' was changed}}
    i |= 2; // expected-note {{variable 'i' with type 'int' was changed}}
    i ^= 2; // expected-note {{variable 'i' with type 'int' was changed}}
    i &= 1; // expected-note {{variable 'i' with type 'int' was changed}}

    int j = 0;
    i = // expected-note {{variable 'i' with type 'int' was changed}}
        ++j; // expected-note {{variable 'j' with type 'int' was changed}}
}

void change_in_for_loop() {
    for ( int i = 0
        ; i < 10
        ; ++i ) // expected-note {{variable 'i' with type 'int' was changed}}
    ;
}

void declaration_statements() {
    int i, j;
    i = j; // expected-note {{variable 'i' with type 'int' was changed}}
}
