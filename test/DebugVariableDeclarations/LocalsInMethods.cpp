// RUN: %clang_verify %show_variables %s

class A {

    int m_i; // expected-note {{variable 'm_i' declared here}}
    int m_j; // expected-note {{variable 'm_j' declared here}}

    A();

    void f1() const;
};

A::A()
    : m_i(0)
    , m_j(1)
{
    int k = m_i; // expected-note {{variable 'k' declared here}}
}

void A::f1() const {
    int k = m_i; // expected-note {{variable 'k' declared here}}
}
