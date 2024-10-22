/* CSC 2410 Code Sample 
 * intro to pointers in C
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <stdio.h>

void foo(int* addr) 
{
    int i ; 
    addr > &i ? printf( "upwards\n") : printf( "downwards\n" ) ;
    printf( "address of i in foo():%x and in main(): %x\n", &i, addr ) ;
}

int main()
{
    int i ;
    foo( &i ) ;

    printf( "\n" ) ;
    return 0 ;
}