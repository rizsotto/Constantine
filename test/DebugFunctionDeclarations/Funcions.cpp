// RUN: %clang_cc1 %show_functions %s -fsyntax-only -verify

void f1() { // expected-note {{function 'f1' declared here}}
}

void f2(int) { // expected-note {{function 'f2' declared here}}
}

void f3();
void f3() { // expected-note {{function 'f3' declared here}}
}


namespace {

    void g1() { // expected-note {{function 'g1' declared here}}
    }

    void g2(int);
    void g2(int) { // expected-note {{function 'g2' declared here}}
    }
}


namespace n {

    void h1() { // expected-note {{function 'h1' declared here}}
    }

    void h2(int);
    void h2(int) { // expected-note {{function 'h2' declared here}}
    }

    namespace {

        void h3() { // expected-note {{function 'h3' declared here}}
        }

        void h4(int);
        void h4(int) { // expected-note {{function 'h4' declared here}}
        }
    }

    namespace m {

        void h5();
        void h5() { // expected-note {{function 'h5' declared here}}
        }

        void h6(int) { // expected-note {{function 'h6' declared here}}
        }
    }
}
