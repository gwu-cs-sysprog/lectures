/* CSC 2410 Code Sample 
 * Function Pointers (generic bubble sort)
 * (c) Sibin Mohan
 * Fall 2024
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


// FUNCTION POINTERS
#define ARRAY_SIZE 10

// typedef int aaaa ;

typedef int (*is_greater_than)(void*, void *) ;
typedef void (*swap)( void*, void* ) ;


// Sort:
// 1. Compare
// 2. Swap
// Sorting ints
void generic_bubble_sort( void* array, int array_size, int element_size, 
                          is_greater_than my_comparator,
                          swap my_swap )
{
    for( unsigned int i = 0 ; i < array_size-1 ; ++i )
        for( unsigned int j = 0 ; j < i ; ++j )
            if( my_comparator(array+(j*element_size), 
                              array+((j+1)*element_size)) ) // COMPARISON
                my_swap(array+(j*element_size), 
                              array+((j+1)*element_size)) ;
}

int is_greater_than_int( void* l, void* r )
{
    int* left = l ;
    int* right = r ;

    if( *left > *right )
        return 1 ;
    else
        return 0 ; 

   // return ( *l > *r ? 1 : 0 ) ;
}

void swap_int( void* l, void* r )
{
    int* left = l ;
    int* right = r ;
    int temp = *left ;

    *left = *right ;
    *right = temp ;
}

int is_greater_than_double( void* l, void* r )
{
    double* left = l ;
    double* right = r ;

    if( *left > *right )
        return 1 ;
    else
        return 0 ; 
}

void swap_double( void* l, void* r )
{
    double* left = l ;
    double* right = r ;
    double temp = *left ;
    *left = *right ;
    *right = temp ;
}


// Sorting ints
void bubble_sort_int( int array[], int array_size )
{
    for( unsigned int i = 0 ; i < array_size-1 ; ++i )
    {
        // int k = array[i] ;
        for( unsigned int j = 0 ; j < i ; ++j )
        {
            if( is_greater_than_int(array+j, array+j+1) )
            {
                swap_int(array+j, array+j+1) ;
            }
        }
    }
}

// Sorting doubles
/*
void bubble_sort_doubles( double array[], int array_size )
{
    for( unsigned int i = 0 ; i < array_size-1 ; ++i )
    {
        // int k = array[i] ;
        for( unsigned int j = 0 ; j < i ; ++j )
        {
            if( is_greater_than_double(array+j, array+j+1) )
            {
                swap_doubles(array+j, array+j+1) ;
            }
        }
    }
}
*/

void print_array_int( int array[], int array_size )
{
    printf( "int array: " ) ;
    for( unsigned int i = 0 ; i < array_size ; ++i )
        printf( "%d ", array[i] ) ;
}

void print_array_double( double array[], int array_size )
{
    printf( "double array: " ) ;
    for( unsigned int i = 0 ; i < array_size ; ++i )
        printf( "%f ", array[i] ) ;
}


int main()
{
    // int my_array[] = { 1, 2, 3, 4, 5 } ;
    int my_array_int[] = { 2341, 8632, 3, 2344, 747645 } ;
    int array_size = 5 ;

    //integer version
    generic_bubble_sort( my_array_int, array_size, sizeof(int), is_greater_than_int, swap_int ) ;

    double my_double_array[] = {1.0, 9485.2, 34.567, 9383.243, 44.1 } ;
    generic_bubble_sort( my_double_array, array_size, sizeof(double), is_greater_than_double, swap_double ) ;


    // bubble_sort( my_array, array_size ) ;

    print_array_int( my_array_int, array_size ) ;
    printf( "\n" ) ;
    print_array_double( my_double_array, array_size ) ;


    printf( "\n" ) ;
}
