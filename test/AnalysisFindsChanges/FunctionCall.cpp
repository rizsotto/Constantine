// RUN: %clang_cc1 %change %s -fsyntax-only -verify

// ..:: fixtures ::..
int inc(int k)
{ return k; }

int inc_ref(int & k)
{ return k; }

int inc_const_ref(int const & k)
{ return k; }

int inc_p(int * const k)
{ return *k; }

int inc_const_p(int const * const k)
{ return *k; }
// ..:: fixtures ::..

void function_call_test() {
    int i = 0;

    inc(i);
    inc_ref(i); // expected-note {{variable 'i' was changed}}
    inc_const_ref(i);
    inc_p(&i); // expected-note {{variable 'i' was changed}}
    inc_const_p(&i);

    int iref = i;

    inc(iref);
    inc_ref(iref); // expected-note {{variable 'iref' was changed}}
    inc_const_ref(iref);
    inc_p(&iref); // expected-note {{variable 'iref' was changed}}
    inc_const_p(&iref);

    int * iptr = &i;

    inc(*iptr);
    inc_ref(*iptr); // expected-note {{variable 'iptr' was changed}}
    inc_const_ref(*iptr);
    inc_p(iptr); // expected-note {{variable 'iptr' was changed}}
    inc_const_p(iptr);
}
