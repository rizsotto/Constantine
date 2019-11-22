// RUN: %verify_variable_changes %s

struct TestType {
    TestType() { }
    TestType(TestType const &) { }
   ~TestType() { }
    TestType & operator=(TestType const &) { return *this; }
};

void test_simple_assignment() {
    TestType const T;
    TestType K;
    K = T; // expected-note {{variable 'K' with type 'struct TestType' was changed}}
}

void test_simple_assignment_with_temporary() {
    TestType K;
    K = TestType(); // expected-note {{variable 'K' with type 'struct TestType' was changed}}
}

void test_assignment_on_pointer() {
    TestType * K = new TestType();
    *K = TestType(); // expected-note {{variable 'K' with type 'struct TestType' was changed}}
}

void test_on_delete() {
    TestType * K = new TestType();
    delete K;
}
