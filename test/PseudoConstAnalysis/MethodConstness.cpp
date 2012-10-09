// RUN: %clang_cc1 %s -fsyntax-only -verify

struct BaseOne {
    int value;

    BaseOne();

    virtual int vf1(int, int);
    int f1(int);

    int f2();

    int cf3() const;
    int f3();

    int f4() const;
    static int sf1();

    int f5() const;
};

struct BaseTwo {
    virtual int vg1(int) = 0;
};

struct Sub : public BaseOne, public BaseTwo {
    int vf1(int, int);
    int vg1(int);

    virtual int vh1(int);

    int h2();
    int h3();
    int h4();
    int h5();
};


BaseOne::BaseOne()
    : value(0)
{ }

int BaseOne::vf1(int i, int j) {
    return value + i + j;
}

int BaseOne::f1(int const i) { // expected-warning {{function 'f1' could be declared as const}}
    return value + i;
}

int BaseOne::f2() {
    return vf1(1, 2);
}

int BaseOne::cf3() const {
    return value;
}

int BaseOne::f3() { // expected-warning {{function 'f3' could be declared as const}}
    return cf3();
}

int BaseOne::f4() const { // expected-warning {{function 'f4' could be declared as static}}
    return 8;
}

int BaseOne::f5() const {
    BaseOne const Value = *this;
    return 8;
}

int BaseOne::sf1() {
    return 8;
}

int Sub::vf1(int i, int j) {
    return value + i + j;
}

int Sub::vg1(int i) {
    return i;
}

int Sub::vh1(int i) {
    return value + i;
}

int Sub::h2() {
    return f2();
}

int Sub::h3() {
    return ++value;
}

int Sub::h4() { // expected-warning {{function 'h4' could be declared as const}}
    return value;
}

int Sub::h5() { // expected-warning {{function 'h5' could be declared as static}}
    return sf1();
}
