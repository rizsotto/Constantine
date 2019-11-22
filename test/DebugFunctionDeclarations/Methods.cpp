// RUN: %show_functions %s

class A {

    void f1() { // expected-note {{function 'f1' declared here}}
    }

    void f2(int) { // expected-note {{function 'f2' declared here}}
    }

    void f3(unsigned int);

    void f4() const;
};

void A::f3(unsigned int) { // expected-note {{function 'f3' declared here}}
}

void A::f4() const { // expected-note {{function 'f4' declared here}}
}


namespace {

    class B {
        void g1() { // expected-note {{function 'g1' declared here}}
        }

        void g2();
    };

    void B::g2() { // expected-note {{function 'g2' declared here}}
    }
}

namespace n {

    class C {
        void h1() { // expected-note {{function 'h1' declared here}}
        }

        void h2(int) { // expected-note {{function 'h2' declared here}}
        }
    };

    namespace {

        class D {
            void h3() { // expected-note {{function 'h3' declared here}}
            }

            void h4(int) { // expected-note {{function 'h4' declared here}}
            }
        };
    }

    namespace m {

        class E {
            void h5() { // expected-note {{function 'h5' declared here}}
            }

            void h6(int) { // expected-note {{function 'h6' declared here}}
            }
        };
    }
}

void wrapper_method() { // expected-note {{function 'wrapper_method' declared here}}

    class F {
        void h7() { // expected-note {{function 'h7' declared here}}
        }

        void h8(int) { // expected-note {{function 'h8' declared here}}
        }
    };
}
