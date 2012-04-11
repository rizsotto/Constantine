#include <stdio.h>

int good_candidate( int k )
{
    int j = k + 1;
    return j;
}

int another_method()
{
    int const j = 0;
    int k = good_candidate( j + 1 );

    return 2 * k;
}
