// RUN: %clang_cc1 %s %usage -fsyntax-only -verify

void simple_type_usage() {
    int const k = 0;
    int const j = k + 1; // expected-note {{variable 'k' was used}}
}

void simple_type_usage_via_function_call() {
    struct Fixture {
        static int inc(int) { return 5; }
    };
    int const k = 0;
    int const j = Fixture::inc(k); // expected-note {{variable 'k' was used}}
}

void simple_type_usage_in_for() {
    for ( int i = 0
        ; i <= 10 // expected-note {{variable 'i' was used}}
        ; ++i) // expected-note {{variable 'i' was used}}
    ;
}

void defined_type_usage() {
    struct Fixture {
    };

    Fixture f;
    Fixture c = f; // expected-note {{variable 'f' was used}}
}

void defined_type_usage_via_member_access() {
    struct Fixture {
        int i;
    };
    Fixture f;
    const int k = f.i + 1; // expected-note {{variable 'f' was used}}
}

void defined_type_usage_via_member_function_call() {
    struct Fixture {
        int get() { return 0; }
    };
    Fixture f;
    const int k = f.get() + 1; // expected-note {{variable 'f' was used}}
}

void defined_type_usage_via_function_call() {
    struct Fixture {
        static int convert(Fixture &) { return 0; }
    };
    Fixture f;
    const int k = Fixture::convert(f); // expected-note {{variable 'f' was used}}
}
