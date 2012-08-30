// RUN: %clang_cc1 %s -fsyntax-only -verify

namespace {
    int a_k = 9; // expected-warning {{variable 'a_k' could be declared as const}}
}

namespace zone {
    int k = 9; // expected-warning {{variable 'k' could be declared as const}}
}

namespace zone {
  namespace inner {
    int k = 9; // expected-warning {{variable 'k' could be declared as const}}
  }
}

int use_anonymous() { return a_k; }
int use_zone() { return zone::k; }
int use_inner_zone() { return zone::inner::k; }
