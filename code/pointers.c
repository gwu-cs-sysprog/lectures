/*
 * CSC 2410 Code Sample 
 * basics of pointers in C
 * Fall 2024
 * (c) Sibin Mohan
 * 
 * Sept. 12, 2024.
 */

#include <stdio.h>

int main()
{
    int* temp = malloc( 239847 ) ;
    int* p_int = (int*) malloc( sizeof(int) * 100 ) ;

    /*
    int i = 100 ;
    // int* p_int = &i ;
    int* p_int ;
    p_int = &i ;

    printf( "i = %d\t addr of i = %p \t value at mem addr %p = %d\n", 
            i, p_int, p_int, *p_int ) ;

    *p_int = 4564 ;
    // p_int = 4564 ;

    printf( "value of i = %d, value at %p = %d\n", i, p_int, *p_int ) ;

    const int ci = 100 ;
    const double pi = 3.14 ;
    const double* const p_pi = &pi ;


    printf ( "BEFORE pi = %f \t *p_pi = %f\n\n", pi, *p_pi ) ;

    // *p_pi = 700.0 ;

    const double e = 2.712 ;
    p_pi = &e ;

    printf ( "AFTER pi = %f \t *p_pi = %f\n", pi, *p_pi ) ;

    // pi = 700 ;
    */

    printf( "\n") ;
    return 0 ;
}