// RUN: %clang_cc1 %change %s -fsyntax-only -verify

// ..:: fixtures ::..
struct A {
    int Value;

    int getValue() const;
    void setValue(int i);

    A & SetValue(int i);
    void GetValue(int &) const;

    void MutantSet(int &);
};
// ..:: fixtures ::..

void member_access_test() {

    A a;

    {
        int i = a.Value;
        a.Value = 3; // expected-note {{variable 'a' with type 'struct A' was changed}} // expected-note {{variable 'Value' with type 'int' was changed}}
    }

    {
        int i = a.getValue();
        a.setValue(2); // expected-note {{variable 'a' with type 'struct A' was changed}}
    }

    {
        int i = 0;
        a.GetValue(i); // expected-note {{variable 'i' with type 'int' was changed}}
        a.SetValue(i); // expected-note {{variable 'a' with type 'struct A' was changed}}
    }

    {
        int i = 0;
        a.MutantSet(i); // expected-note {{variable 'a' with type 'struct A' was changed}} // expected-note {{variable 'i' with type 'int' was changed}}
    }
}
