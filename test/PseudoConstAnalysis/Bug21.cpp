// RUN: %clang_verify %s
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
