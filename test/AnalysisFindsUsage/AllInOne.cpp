// RUN: %clang_cc1 %usage %s -fsyntax-only -verify

void simple_type_usage() {
    int const k = 0;
    int const j = k + 1; // expected-note {{symbol 'k' was used}}
}

void simple_type_usage_via_function_call() {
    struct Fixture {
        static int inc(int) { return 5; }
    };
    int const k = 0;
    int const j = Fixture::inc(k); // expected-note {{symbol 'k' was used}} // expected-note {{symbol 'inc' was used}}
}

void simple_type_usage_in_for() {
    for ( int i = 0
        ; i <= 10 // expected-note {{symbol 'i' was used}}
        ; ++i) // expected-note {{symbol 'i' was used}}
    ;
}

void defined_type_usage() {
    struct Fixture {
    };

    Fixture f;
    Fixture c = f; // expected-note {{symbol 'f' was used}}
}

void defined_type_usage_via_member_access() {
    struct Fixture {
        int i;
    };
    Fixture f;
    const int k = f.i + 1; // expected-note {{symbol 'f' was used}}
}

void defined_type_usage_via_member_function_call() {
    struct Fixture {
        int get() { return 0; }
    };
    Fixture f;
    const int k = f.get() + 1; // expected-note {{symbol 'f' was used}}
}

struct CommonFixture {
    int i;
    int seti(int j) {
        i = j; // expected-note {{symbol 'i' was used}} // expected-note {{symbol 'j' was used}}
        return i; // expected-note {{symbol 'i' was used}}
    }
    int candidate() { return seti(2); } // expected-note {{symbol 'seti' was used}}
};

void defined_type_usage_via_function_call() {
    struct Fixture {
        static int convert(Fixture &) { return 0; }
    };
    Fixture f;
    const int k = Fixture::convert(f); // expected-note {{symbol 'f' was used}} // expected-note {{symbol 'convert' was used}} 
}
