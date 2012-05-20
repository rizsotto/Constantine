// RUN: %clang_cc1 %s -fsyntax-only -verify

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


void test_method()
{
    {
        Simple s; // expected-warning {{variable could be declared as const [Medve plugin]}}
        int const k = s.getId();
    }
    {
        Simple s;
        s.setId(2);
    }
}
