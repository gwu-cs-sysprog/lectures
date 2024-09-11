/*
 * CSC 2410 Code Sample 
 * basic types in C
 * Fall 2024
 * (c) Sibin Mohan
 * 
 * Sept. 05, 2023.
 */

#include <stdio.h>

enum weekday{ sunday=1, monday, tuesday, wednesday, thursday, friday, saturday } ;

struct calendar{
    int _day ;
    int _month ;
    int _year ;
    // int _day_of_week ;
    // enum weekday _day_of_week ;
    char _day_of_week[16] ;
} ;

union my_info
{
    unsigned int _age ; // 4 bytes
    unsigned int _weight ; // 8 by\tes
} ;

// 1 - sunday, 2 - monday ...
// "thursday"

int main()
{
    union my_info sibin ;

    printf( "Size of union = %lu\n", sizeof(sibin) ) ;
    sibin._age = 2342341 ;
    printf( "age = %d\t weight = %d\n", sibin._age, sibin._weight ) ;

    sibin._weight= 234.1234234 ;
    printf( "age = %d\t weight = %d\n", sibin._age, sibin._weight ) ;
    /*
    char c = 'x' ;
    int i = 100 ;
    float f ;
    double d ;

    i = 100 ;
    f = 1.1 ;
    d = 12343.48484 ;

    printf( "Memory sizes of variables: \n\n \
             size of char = %lu\n \
             size of int = %lu\n \
             size of float = %lu \n \
             size of double = %lu \n",
             sizeof(c), sizeof(i), sizeof(f), sizeof(d) ) ;
    */

   /*
    // struct calendar today = { 5, 9, 2024 } ;
    struct calendar today = { 5, 9, 2024, "thursday" } ;
    // today._day = 5 ;
    // today._month = 9 ;
    // today._year = 2024 ;
    // today._day_of_week = 5 ;
    // today._day_of_week = thursday ;
    // today._day_of_week = "thursday" ;

    today._day_of_week[0] = 't' ;
    today._day_of_week[1] = 'u' ;
    today._day_of_week[2] = 'e' ;
    today._day_of_week[3] = 's' ;
    today._day_of_week[4] = 'd' ;
    today._day_of_week[5] = 'a' ;
    today._day_of_week[6] = 'y' ;
    // today._day_of_week[7] = '\0' ;

    printf( "size of the struct = %lu\n", sizeof(today) ) ;

    printf( "date: %d/%d/%d \n", 
            today._month, today._day, today._year ) ;
            
    printf( "day of week = %s\n", today._day_of_week ) ;

    // char hello[10] = "hello" ;
    // char hello[10] ;
    // hello = "hi!" ;
    */

    printf( "\n" ) ;
    return 0 ;
}
