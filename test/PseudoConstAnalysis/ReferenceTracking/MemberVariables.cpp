// RUN: %clang_cc1 %s -fsyntax-only -verify

struct TestType {
    int m_i;

    void do_mutating_through_reference() {
        int & i = m_i;
        ++i;
    }

    void mutated_by_value_but_reference_reported() {
        int & i = m_i; // expected-warning {{variable 'i' could be declared as const}}
        ++m_i;
    }

    void do_mutating_through_pointer() {
        int * i = &m_i;
        ++(*i);
    }

    void mutated_by_value_but_pointer_reported() {
        int * i = &m_i; // expected-warning {{variable 'i' could be declared as const}}
        ++m_i;
    }
};
