// RUN: %clang_cc1 %s -fsyntax-only -verify
// expected-no-diagnostics

struct Functor {
    int const mi;

    Functor()
        : mi(0)
    { }

    void operator()(int & i) const {
        i = mi;
    }
};

void test_1() {
    int i = 0;
    Functor()(i);
}
