// RUN: %clang_cc1 %s -fsyntax-only -verify

void pointer_test()
{
    int k = 0;

    int * p1 = &k; // expected-warning {{variable could be declared as const [Medve plugin]}}
    int * const p2 = &k;
    int const * p3 = &k; // expected-warning {{variable could be declared as const [Medve plugin]}}
}

void double_pointer_test()
{
    int k = 0;

    int * const p1 = &k;
    int * const * pp1 = &p1; // expected-warning {{variable could be declared as const [Medve plugin]}}
}

