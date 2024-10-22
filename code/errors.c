/* CSC 2410 Code Sample 
 * Error Handling in C/UNIX
 * Fall 2024
 * (c) Sibin Mohan
 */

// if( strcpy(dest, src) )

#include <stdio.h>
#include <error.h>
#include <errno.h> 
#include <string.h>
#include <limits.h>
#include <stdlib.h>

int main()
{
    /*
    int n = printf( "What am I doing here?" ) ;

    printf( "\n the previous printf() returned = %d\n", n ) ;
    */

    // create an array of size 9223372036854775807 bytes!
    char* massive_array = (char*) malloc( LONG_MAX ) ;

    if( massive_array == NULL )
    {
        // the malloc() failed!
        printf( "errno = %d\n", errno ) ;
        perror( "Too much memory requested!" ) ;
    }

    errno = 100 ;
    char* error_string = strerror( errno ) ;
    printf( "String Error = %s\n", error_string ) ;

    printf( "\n" ) ;
    return 0 ;
}

/*
 * In-Class Exercise. Print out all the error codes and strings in the following format:

Error Number    String Error
-----------------------------
1               Operation not permitted
2               No such file or directory
...

*/