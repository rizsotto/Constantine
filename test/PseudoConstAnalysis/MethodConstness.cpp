// RUN: %clang_cc1 %s -fsyntax-only -verify

struct Base {
    int const value;

    Base();

    virtual int vf1(int, int);
    int f1(int);
};

struct Sub : public Base {
    int vf1(int, int);

    virtual int vf2(int);
};


Base::Base()
    : value(0)
{ }

int Base::vf1(int i, int j) {
    return value + i + j;
}

int Base::f1(int const i) { // expected-warning {{function 'f1' could be declared as const}}
    return value + i;
}

int Sub::vf1(int i, int j) {
    return value + i + j;
}

int Sub::vf2(int i) {
    return value + i;
}
