// RUN: %clang_cc1 %s -fsyntax-only -verify

int inc(int k)
{ return ++k; }

int inc_ref(int & k)
{ return ++k; }

int inc_const_ref(int const & k)
{ return k + 1; }

int inc_p(int * const k)
{ return ++(*k); }

int inc_const_p(int const * const k)
{ return *k + 1; }

namespace {
    void other_method()
    {
        { int k = 1; inc(k); } // expected-warning {{variable could be declared as const [Medve plugin]}}
        { int k = 1; inc_ref(k); }
        { int k = 1; inc_const_ref(k); } // expected-warning {{variable could be declared as const [Medve plugin]}}
        { int k = 1; inc_p(&k); }
        { int k = 1; inc_const_p(&k); }
    }
}
