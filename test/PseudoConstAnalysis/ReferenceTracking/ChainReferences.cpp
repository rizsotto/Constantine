// RUN: %clang_cc1 %s -fsyntax-only -verify

void test() {
    int i = 0;
    int & k = i;
    int * j = &k;

    ++(*j);
}
