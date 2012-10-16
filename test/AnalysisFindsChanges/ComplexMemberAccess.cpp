// RUN: %clang_cc1 %change %s -fsyntax-only -verify

// ..:: fixtures ::..
struct A {
    int Value;

    int GetValue() const {
        return Value;
    }

    void SetValue(int const & i) {
        Value = i; // expected-note {{variable 'Value' with type 'int' was changed}}
    }
};

struct B {
    A Inside;
};

void change_by_ref(B &);
void change_by_ref(A &);

void change_by_ptr(B *);
void change_by_ptr(A *);
// ..:: fixtures ::..

void member_access_test() {

    B b;
    A & a = b.Inside;

    change_by_ref(b); // expected-note {{variable 'b' with type 'struct B' was changed}}
    change_by_ref(b.Inside); // expected-note {{variable 'b' with type 'struct A' was changed}}

    change_by_ptr(&b); // expected-note {{variable 'b' with type 'struct B' was changed}}
    change_by_ptr(&(b.Inside)); // expected-note {{variable 'b' with type 'struct A' was changed}}

    change_by_ref(a); // expected-note {{variable 'a' with type 'struct A' was changed}}
    change_by_ptr(&a); // expected-note {{variable 'a' with type 'struct A' was changed}}

    a.SetValue(0); // expected-note {{variable 'a' with type 'struct A' was changed}}
    b.Inside.SetValue(0); // expected-note {{variable 'b' with type 'struct B' was changed}}

    b.Inside.Value += 1; // expected-note {{variable 'b' with type 'struct B' was changed}}
    a.Value += 1; // expected-note {{variable 'a' with type 'struct A' was changed}}
}
