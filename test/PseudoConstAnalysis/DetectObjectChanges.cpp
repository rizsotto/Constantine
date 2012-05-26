// RUN: %clang_cc1 %s -fsyntax-only -verify

// ..:: fixture begin ::..

class Simple {
    int m_id;
public:
    Simple();
   ~Simple();

    int getId() const;
    Simple & setId(int);

private:
    Simple(Simple const &);
    Simple & operator=(Simple const &);
};

Simple::Simple()
    : m_id(0)
{ }

Simple::~Simple()
{ }

int Simple::getId() const
{ return m_id; }

Simple & Simple::setId(int const id)
{ m_id = id; return *this; }

void change(int & k)
{ ++k; }

void dont_change(int const & k)
{ }

// ..:: fixture end ::..

void test_method_call() {
    {
        Simple s; // expected-warning {{variable could be declared as const [Medve plugin]}}
        int const k = s.getId();
    }
    {
        Simple s;
        s.setId(2);
    }
}

void test_method_call_through_reference() {
    {
        Simple s; // expected-warning {{variable could be declared as const [Medve plugin]}}
        Simple const & k = s;
        int const l = k.getId();
    }
    {
        Simple s;
        Simple & k = s;
        k.setId(3);
    }
}


void test_member_access() {

    struct Public {
        int m_id;
    };

    {
        Public s = { 2 }; // expected-warning {{variable could be declared as const [Medve plugin]}}
        int const id = s.m_id;
    }
    {
        Public s = { 2 };
        s.m_id = 3;
    }
    {
        Public s = { 2 }; // expected-warning {{variable could be declared as const [Medve plugin]}}
        int const & k = s.m_id;
    }
    {
        Public s = { 2 };
        int & k = s.m_id;
        k = 3;
    }
    {
        Public s = { 2 }; // expected-warning {{variable could be declared as const [Medve plugin]}}
        dont_change( s.m_id );
    }
    {
        Public s = { 2 };
        change( s.m_id );
    }
}
