// RUN: %clang_cc1 %s -fsyntax-only -verify
// expected-no-diagnostics

struct B {
    int member;

    int mutator() {
        ++member;
        return member;
    }

    int const_logic(int const arg) const {
        return arg + member;
    }

    int test() {
        return const_logic(mutator());
    }
};
