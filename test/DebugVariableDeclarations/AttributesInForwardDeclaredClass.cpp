// RUN: %clang_cc1 %show_variables %s -fsyntax-only -verify

class ForwardDeclared;

class ForwardDeclared {

    int m_value; // expected-note {{variable 'm_value' declared here}}

    int getValue() const;
};


int ForwardDeclared::getValue() const
{
    return m_value;
}
