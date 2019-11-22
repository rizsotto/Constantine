// RUN: %show_functions %s

template <typename T>
void f1() { // expected-note {{function 'f1' declared here}}
}

template <typename T>
void f2(int) { // expected-note {{function 'f2' declared here}}
}

template <typename T>
void f3();

template <typename T>
void f3() { // expected-note {{function 'f3' declared here}}
}


namespace {

    template <typename T>
    void g1() { // expected-note {{function 'g1' declared here}}
    }

    template <typename T>
    void g2(int);

    template <typename T>
    void g2(int) { // expected-note {{function 'g2' declared here}}
    }
}


namespace n {

    template <typename T>
    void h1() { // expected-note {{function 'h1' declared here}}
    }

    template <typename T>
    void h2(int);

    template <typename T>
    void h2(int) { // expected-note {{function 'h2' declared here}}
    }

    namespace {

        template <typename T>
        void h3() { // expected-note {{function 'h3' declared here}}
        }

        template <typename T>
        void h4(int);

        template <typename T>
        void h4(int) { // expected-note {{function 'h4' declared here}}
        }
    }

    namespace m {

        template <typename T>
        void h5();

        template <typename T>
        void h5() { // expected-note {{function 'h5' declared here}}
        }

        template <typename T>
        void h6(int) { // expected-note {{function 'h6' declared here}}
        }
    }
}
