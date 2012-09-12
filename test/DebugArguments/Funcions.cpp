// RUN: %clang_cc1 %s %show_arguments -fsyntax-only -verify

void f1() {
}

void f2(int k) { // expected-note {{variable 'k' declared here}}
}

void f3(int);
void f3(int k) { // expected-note {{variable 'k' declared here}}
}

void f4(int j);
void f4(int k) { // expected-note {{variable 'k' declared here}}
}
