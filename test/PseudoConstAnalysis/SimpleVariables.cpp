// RUN: %clang_cc1 %s -fsyntax-only -verify

int inc(int k)
{ return ++k; }

void big_test_method()
{
    { int k; ++k; }
    { int k; --k; }
    { int k; k++; }
    { int k; k--; }
    { int k; int * const j = &k; *j = 2; }
    { int k = 0; k = 1; }
    { int k = 0; k = k; } // expected-warning {{variable could be declared as const [Medve plugin]}}
    { int k = 2; k += 2; }
    { int k = 2; k -= 2; }
    { int k = 2; k *= 2; }
    { int k = 2; k /= 2; }
    { int k = 2; k &= 0xf0; }
    { int k = 2; k |= 0xf0; }
    { int k = 0; int & j = k; ++j; }
    { int k = 1; inc(k); } // expected-warning {{variable could be declared as const [Medve plugin]}}
    { int const k = 2; inc(k); }
}

namespace {
    void test_method_in_namespace()
    { int k = 1; inc(k); } // expected-warning {{variable could be declared as const [Medve plugin]}}
}

void reference_to_reference_test()
{
    int k = 1;
    int & j = k;
    int & l = j; // expected-warning {{variable could be declared as const [Medve plugin]}}
    int const m = inc(l);
}
