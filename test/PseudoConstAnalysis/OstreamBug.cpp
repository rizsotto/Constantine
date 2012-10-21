// RUN: %clang_cc1 %s -fsyntax-only -verify
// expected-no-diagnostics

class Ostream {
public:
    void put(int const i) { m_i += i; }

private:
    int m_i;
};

Ostream & operator<<(Ostream & os, int const i) {
    os.put(i);

    return os;
}

Ostream & operator<<(Ostream & os, char const c) {
    int const i = c;
    os.put(i);

    return os;
}

// this one is triggering false positive warning
void test(Ostream & os, char const * const text) {
    os << text[0];
}
