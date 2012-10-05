// RUN: %clang_cc1 %s -fsyntax-only -verify

struct Base {
    int const value;

    Base();

    virtual int vf1(int, int) const;
    int f1(int) const;
};

struct Sub : public Base {
    int vf1(int, int) const;

    virtual int vf2(int) const;
};


Base::Base()
    : value(0)
{ }

int Base::vf1(int i, int j) const {
    return value + i + j;
}

int Base::f1(int i) const { // expected-warning {{variable 'i' could be declared as const}}
    return value + i;
}

int Sub::vf1(int i, int j) const {
    return value + i + j;
}

int Sub::vf2(int i) const {
    return value + i;
}
