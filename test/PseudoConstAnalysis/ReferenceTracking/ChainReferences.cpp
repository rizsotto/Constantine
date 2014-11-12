// RUN: %clang_verify %s
// expected-no-diagnostics

void test() {
    int i = 0;
    int & k = i;
    int * j = &k;

    ++(*j);
}
