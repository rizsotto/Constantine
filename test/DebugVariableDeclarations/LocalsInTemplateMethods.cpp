// RUN: %show_variables %s

template <typename T>
class A {

    T m_i; // expected-note {{variable 'm_i' declared here}}
    T m_j; // expected-note {{variable 'm_j' declared here}}

    A();

    template <typename S>
    void f1() const;
};

template <typename T>
A<T>::A()
    : m_i(0)
    , m_j(1)
{
    T k = m_i; // expected-note {{variable 'k' declared here}}
}

template <typename T>
template <typename S>
void A<T>::f1() const {
    T k = m_i; // expected-note {{variable 'k' declared here}}
}
