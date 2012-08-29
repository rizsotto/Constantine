// RUN: %clang_cc1 %s -fsyntax-only -verify

// ..:: fixture begin ::..
namespace {

void change_argument_via_reference(int & k)
{ ++k; }

int use_argument_via_const_reference(int const & k)
{ return (k == 9) ? 1 : 2; }

} // namespace anonmous
// ..:: fixture end ::..

void member_access_test() {

    struct Public {
        int m_id;
    };

    {
        Public s = { 2 }; // expected-warning {{variable could be declared as const}}
        int const id = s.m_id;
    }
    {
        Public s = { 2 };
        s.m_id = 3;
    }
    {
        Public s = { 2 }; // expected-warning {{variable could be declared as const}}
        int const & k = s.m_id;
    }
    {
        Public s = { 2 };
        int & k = s.m_id;
        k = 3;
    }
    {
        Public s = { 2 }; // expected-warning {{variable could be declared as const}}
        use_argument_via_const_reference( s.m_id );
    }
    {
        Public s = { 2 };
        change_argument_via_reference( s.m_id );
    }
}

void method_call_test() {

    struct Simple {
        Simple()
        : m_id(0)
        { }

        int getId() const {
            return m_id;
        }

        Simple & setId(int const id) {
            m_id = id;
            return *this;
        }

    private:
        int m_id;
    };

    // by value
    {
        Simple s; // expected-warning {{variable could be declared as const}}
        int const k = s.getId();
    }
    {
        Simple const s;
        int const k = s.getId();
    }
    {
        Simple s;
        s.setId(2);
    }
    // by reference
    {
        Simple s; // expected-warning {{variable could be declared as const}}
        Simple const & k = s;
        int const l = k.getId();
    }
    {
        Simple s; // would be good
        Simple & k = s; // expected-warning {{variable could be declared as const}}
        int const l = k.getId();
    }
    {
        Simple s;
        Simple & k = s;
        k.setId(3);
    }
    // by pointer
    {
        Simple s; // would be good
        Simple const * const k = &s;
        int const l = k->getId();
    }
    {
        Simple s; // would be good
        Simple * const k = &s; // would be good
        int const l = k->getId();
    }
    {
        Simple s;
        Simple * const k = &s;
        k->setId(3);
    }
}
