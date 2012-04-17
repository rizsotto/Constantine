// RUN: %clang_cc1 %s
#include <stdio.h>

int good_candidate( int );

namespace
{

int another_method()
{
    int const j = 0;
    int k = good_candidate( j + 1 );

    return 2 * k;
}

}

void with_attribute() __attribute__ ((deprecated));

int good_candidate( int k )
{
    int j = k + 1;
    return j;
}

class AClass
{
    void example_method( int );
    int const_method( int ) const;
};

void AClass::example_method( int k )
{
    const int * i = & k;
    int * ii = const_cast<int *>( i );
    int * iii = (int *)i;
}

int AClass::const_method( const int k ) const
{
    return good_candidate( k );
}
