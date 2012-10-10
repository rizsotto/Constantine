// RUN: %clang_cc1 %change %s -fsyntax-only -verify

struct SomeType { };

void test_constructor_by_ref() {
    struct TestType {
        TestType(SomeType &) { }
    };

    SomeType T;
    TestType const K(T); // expected-note {{variable 'T' with type 'struct SomeType' was changed}}
}

void test_constructor_by_const_ref() {
    struct TestType {
        TestType(SomeType const &) { }
    };

    SomeType T;
    TestType const K(T);
}

void test_constructor_by_ptr() {
    struct TestType {
        TestType(SomeType *) { }
    };

    SomeType T;
    TestType const K(&T); // expected-note {{variable 'T' with type 'struct SomeType' was changed}}
}

void test_constructor_by_const_ptr() {
    struct TestType {
        TestType(SomeType const *) { }
    };

    SomeType T;
    TestType const K(&T);
}
