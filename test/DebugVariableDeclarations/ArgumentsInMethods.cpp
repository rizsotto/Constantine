// RUN: %clang_cc1 %show_variables %s -fsyntax-only -verify

class A {

    A(int & k) { }

    void f1() {
    }

    void f2(int k) { // expected-note {{variable 'k' declared here}}
    }

    void f3(unsigned int);

    void f4(int a, int b) const;

    static int f5(int k) { // expected-note {{variable 'k' declared here}}
        return 0;
    }
};

void A::f3(unsigned int k) { // expected-note {{variable 'k' declared here}}
}

void A::f4(int i, // expected-note {{variable 'i' declared here}}
           int j) const { // expected-note {{variable 'j' declared here}}
}

class Base {
    virtual int vf1(int k) {
        return k;
    }

    virtual int vf2(int k, int j) = 0;
};

class Sub : public Base {
    int vf2(int k, int j) {
        return k + j;
    }
};
