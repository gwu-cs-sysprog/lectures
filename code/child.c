/* CSC 2410 Code Sample 
 * processes | exec () | child process
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <stdio.h>

int main( int argc, char* argv[] )
{
    if( argc )
    {
        // positive number of args
        // FIRST arg is the program name

        printf( "INSIDE CHILD: Command Line arguments received: " ) ;
        for( unsigned int i = 0 ; i < argc ; ++i )
            printf( "%s \n", argv[i] ) ;
    }

    printf( "\n" ) ;
    return 100 ;
}