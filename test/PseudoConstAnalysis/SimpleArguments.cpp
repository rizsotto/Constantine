// RUN: %clang_cc1 %s -fsyntax-only -verify

int inc(int k)
{ return ++k; }

int dec(int k)
{ return --k; }

int post_inc(int k)
{ k++; return 0; }

int post_dec(int k)
{ k--; return 0; }

int address_of(int k)
{ int * const j = &k; return 0; }

int assign(int k)
{ k = 9; return k; }

int self_assign(int k) // expected-warning {{variable could be declared as const [Medve plugin]}}
{ k = k; return 0; }

int add_assign(int k)
{ k += 1; return 0; }

int sub_assign(int k)
{ k -= 1; return 0; }

int mul_assign(int k)
{ k *= 2; return 0; }

int div_assign(int k)
{ k /= 2; return 0; }

int and_assign(int k)
{ k &= 0x1; return 0; }

int or_assign(int k)
{ k |= 0x1f; return 0; }

int call_inc(int k) // expected-warning {{variable could be declared as const [Medve plugin]}}
{ return inc(k); }

int call_inc_with_const(int const k)
{ return inc(k); }

int ref_declared(int k)
{ int & j = k; ++j; return 0; }
