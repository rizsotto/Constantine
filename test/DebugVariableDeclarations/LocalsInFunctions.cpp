// RUN: %clang_cc1 %show_variables %s -fsyntax-only -verify

void f1() {
    int i = 0; // expected-note {{variable 'i' declared here}}
    int j, k; // expected-note {{variable 'j' declared here}} // expected-note {{variable 'k' declared here}}

    for ( int l = 0 // expected-note {{variable 'l' declared here}}
        ; l < 10
        ; ++l ) {
    }

    if ( int m = i ) { // expected-note {{variable 'm' declared here}}
        int n = k + j; // expected-note {{variable 'n' declared here}}
    }
}
