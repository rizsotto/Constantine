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
    void test_function_call()
    {
        { int konst = 1; inc(konst); } // expected-warning {{variable could be declared as const [Medve plugin]}}
        { int vary = 1; inc_ref(vary); }
        { int konst = 1; inc_const_ref(konst); } // expected-warning {{variable could be declared as const [Medve plugin]}}
        { int vary = 1; inc_p(&vary); }
        { int konst = 1; inc_const_p(&konst); } // would be nice
    }
}
