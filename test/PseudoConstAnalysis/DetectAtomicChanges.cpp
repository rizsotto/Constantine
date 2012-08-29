// RUN: %clang_cc1 %s -fsyntax-only -verify

void binary_operator_test() {
    { int k = 0; k = 1; }
    { int k = 0; k = k; } // expected-warning {{variable could be declared as const}}
    { int k = 2; k += 2; }
    { int k = 2; k -= 2; }
    { int k = 2; k *= 2; }
    { int k = 2; k /= 2; }
    { int k = 2; k &= 0xf0; }
    { int k = 2; k |= 0xf0; }
}

void unary_operator_test() {
    { int k; ++k; }
    { int k; --k; }
    { int k; k++; }
    { int k; k--; }
}
