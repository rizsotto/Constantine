// RUN: %clang_verify %show_functions %s

template <typename T>
class A {

    void f1() { // expected-note {{function 'f1' declared here}}
    }

    void f2(T) { // expected-note {{function 'f2' declared here}}
    }

    void f3(unsigned int);

    template <typename S>
    void f4() const;
};

template <typename T> void A<T>::f3(unsigned int) { // expected-note {{function 'f3' declared here}}
}

template <typename T> template <typename S> void A<T>::f4() const { // expected-note {{function 'f4' declared here}}
}


class B : public A<B> {
    void g1() { // expected-note {{function 'g1' declared here}}
    }

    void g2();
};

void B::g2() { // expected-note {{function 'g2' declared here}}
}

