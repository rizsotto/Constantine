// RUN: %clang_verify %change %s

template<typename T>
struct TestType {
    void change() { }
    void not_change() const { }

    template <typename M>
    void change(M const &) { }

    template <typename M>
    void not_change(M const &) const { }
};

void test_non_const_method() {
    TestType<int> T;
    T.change(); // expected-note {{variable 'T' with type 'TestType<int>' was changed}}
}

void test_const_method() {
    TestType<int> T;
    T.not_change();
}

void test_non_const_template_method() {
    TestType<int> T;
    T.change('k'); // expected-note {{variable 'T' with type 'TestType<int>' was changed}}
}

void test_const_template_method() {
    TestType<int> T;
    T.not_change('k');
}
