// RUN: %clang_cc1 %s %show_local_variables -fsyntax-only -verify

class A {

    int m_i;
    int m_j;

    A();

    void f1(int) const;
};

A::A()
    : m_i(0)
    , m_j(1)
{
    int k = m_i; // expected-note {{variable 'k' declared here}}
}

void A::f1(int a) const {
    int k = m_i; // expected-note {{variable 'k' declared here}}
}
