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
    void function_call_test() {
        { int konst = 1; inc(konst); } // expected-warning {{variable 'konst' could be declared as const}}
        { int vary = 1; inc_ref(vary); }
        { int konst = 1; inc_const_ref(konst); } // expected-warning {{variable 'konst' could be declared as const}}
        { int vary = 1; inc_p(&vary); }
        { int konst = 1; inc_const_p(&konst); } // would be nice
    }
}
