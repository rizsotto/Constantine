// RUN: %clang_cc1 %s -fsyntax-only -verify

struct SomeType {
    SomeType() { }
};

void test_simple_assignment() {
    SomeType const T;
    SomeType K;
    K = T;
}

void test_simple_assignment_with_temporary() {
    SomeType K;
    K = SomeType();
}

void test_complex_member_assignemt() {
    struct Container {
        SomeType T;
        int Value;
    };

    Container Result;
    Result.T = SomeType();
}
