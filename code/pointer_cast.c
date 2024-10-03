/*
 * CSC 2410 Code Sample 
 * basics of pointers in C
 * Fall 2024
 * (c) Sibin Mohan
 * 
 * Sept. 10, 2024.
 * 
 * 1145258561
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main()
{
    int* a = (int*)malloc( sizeof(int) ) ;
    assert( a != NULL ) ; // assert (a) ;
    // DEFENSIVE PROGRAMMING

    *a = 1145258561 ;
    printf( "Value at mem loc pointed by a = %d\n", *a ) ;

    char* pc = (char*) a ; // CASTING FROM INT* to CHAR*
    for( unsigned int i = 0 ; i < sizeof(int) ; ++i )
        printf( "%c ", *pc++ ) ; 
    
    int aa = 100 ;
    // ++aa ; // aa = aa + 1 ;
    int b = ++aa ; // ++a ; b = a 

    aa = 100 ;
    int c = aa++ ; // c = a ; ++a

    printf( "\n aa = %d\t b = %d\t c= %d\n", aa, b, c ) ;

    void* v = a ;

    // *v = 100 ;

    printf( "\n" ) ;
    return 0 ;
}

