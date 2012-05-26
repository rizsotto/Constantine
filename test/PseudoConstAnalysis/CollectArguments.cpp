// RUN: %clang_cc1 %s -fsyntax-only -verify

int inc(int k) {
    return ++k;
}

int call_inc(int k) { // expected-warning {{variable could be declared as const [Medve plugin]}}
    return inc(k);
}

int single_argument(int j) { // expected-warning {{variable could be declared as const [Medve plugin]}}
    switch (j) {
    case 6 :
    case 8 :
        return inc(0);
    default:
        break;
    }
    return 0;
}

int two_arguments(int j, int k) { // expected-warning {{variable could be declared as const [Medve plugin]}}
    switch (j) {
    case 6 :
    case 8 :
        return inc(k);
    default:
        ++k;
        break;
    }
    return 0;
}

int two_const_arguments(int j // expected-warning {{variable could be declared as const [Medve plugin]}}
    , int k) { // expected-warning {{variable could be declared as const [Medve plugin]}}
    switch (j) {
    case 6 :
    case 8 :
        return inc(j);
    default:
        return inc(k);
        break;
    }
    return 0;
}
