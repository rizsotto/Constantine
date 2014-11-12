// RUN: %clang_verify %s

struct EmbededType {
    int m_i;
};

struct TestType {
    EmbededType m_e;

    void do_mutating_through_reference() {
        int & i = m_e.m_i;
        ++i;
    }

    void mutated_by_value_but_reference_reported() {
        int & i = m_e.m_i; // expected-warning {{variable 'i' could be declared as const}}
        ++(m_e.m_i);
    }

    void do_mutating_through_pointer() {
        int * i = &(m_e.m_i);
        ++(*i);
    }

    void mutated_by_value_but_pointer_reported() {
        int * i = &(m_e.m_i); // expected-warning {{variable 'i' could be declared as const}}
        ++(m_e.m_i);
    }
};
