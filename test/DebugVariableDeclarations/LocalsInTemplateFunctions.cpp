// RUN: %clang_verify %show_variables %s

template<typename T>
void f1() {
    T i = 0; // expected-note {{variable 'i' declared here}}
    T j, k; // expected-note {{variable 'j' declared here}} // expected-note {{variable 'k' declared here}}

    for ( T l = 0 // expected-note {{variable 'l' declared here}}
        ; l < 10
        ; ++l ) {
    }

    if ( T m = i ) { // expected-note {{variable 'm' declared here}}
        T n = k + j; // expected-note {{variable 'n' declared here}}
    }
}

void f2() {
    return f1<int>();
}
