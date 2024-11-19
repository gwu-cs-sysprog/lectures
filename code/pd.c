/* CSC 2410 Code Sample 
 * processes | descriptors ()
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define CWD_BUFFER_SIZE 128
#define OUTPUT_BUFFER_SIZE 512

int main()
{
    char* hw = "hello world\n" ; 
    // int fd ;
    ssize_t amt ; // signed integer

    amt = write( STDOUT_FILENO, hw, strlen(hw) ) ;
    printf( "Num of bytes written: %d\n", amt ) ;

    if( amt == 0 )
    {
        printf( "Nothing written!\n" ) ;
        exit( EXIT_FAILURE ) ;
    } 
    else if( amt > 0 )
    {
        assert( amt == (ssize_t)strlen(hw) ) ;

        // ...success! Do other stuff...
    }
    else if( amt < 0 )
    {
        perror( "write failed!!" ) ;
        exit( EXIT_FAILURE ) ;
    }

    amt = write( STDERR_FILENO, hw, strlen(hw) ) ;
    assert( amt == (ssize_t)strlen(hw) ) ;

    int fd = dup(STDOUT_FILENO) ;
    assert( fd >= 0 ) ; // check if dup() succeeded 

    char output[ OUTPUT_BUFFER_SIZE ] ;
    amt = snprintf( output, OUTPUT_BUFFER_SIZE, "default input channel: %d\t \
                default output channel: %d\t default error channel: %d \
                new file descriptor: %d\n", 
                STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, fd ) ;
    
    output[amt] = '\0' ;
    write( STDOUT_FILENO, output, amt ) ;  

    close( STDOUT_FILENO ) ;

    /*
    char cwd[CWD_BUFFER_SIZE] ;

    getcwd( cwd, CWD_BUFFER_SIZE ) ;

    char* cwd = getcwd( NULL, 0 ) ;
    assert(cwd) ; // if this assert fails, something went wrong in getcwd()

    printf( "Current Directory: %s\n", cwd ) ;
    free(cwd) ;

    if (chdir( ".." ) == -1)
    {
        // error!
        perror("Error in chdir()") ;
        abort() ;
    }

    cwd = getcwd( NULL, 0 ) ;
    printf( "Current Directory: %s\n", cwd ) ;
    free(cwd) ;
    */

    printf( "\n" ) ;
    return 0 ;
}
