
#include <stdio.h>
#include <assert.h>

// FORWARD DECLARATIONS
// double celsius_to_fahrenheit = 9/5 ; // IMPLICIT CONVERSION
// extern const double celsius_to_fahrenheit ;
static const double celsius_to_fahrenheit ;

// double foo( double a ) ;
extern void foo(void) ;

// double temp ;

int main()
{

    // printf( "temperature = %f\n", temperature ) ;
    // foo(1.0) ;

    double fahrenheit = 40 * celsius_to_fahrenheit + 32 ;

    printf( "variable fahrenheit = %f\t explicit conversion = %f\n", fahrenheit, ((40*1.8)+32) ) ;

    printf( "\n" ) ;
    return 0 ;
}



/*
double foo( double a )
{
    // double temperature = a ; // LOCAL to foo()

    // DOUBLE
    double temperature = 98.4 ;
    printf( "Outside temperature = %f\n", temperature ) ;

    {
        double temperature = 10 ;
        printf( "Inside temperature = %f\n", temperature ) ;

    }

    // celsius_to_fahrenheit = 1.0 ;


    /*
    if(1)
    {
        int i = 100 ;
    }
     i = 200 ;

    {
        j = 200 ;
    }
// }
*/
