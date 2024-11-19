/* CSC 2410 Code Sample 
 * communication between processes | pipes ()
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <unistd.h> // pipe()
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h> // for MIN
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 16
#define LARGE_BUFFER_SIZE 1<<18
#define WRITE_CHUNK 1<<8  // how much to write in each iteration

int main()
{
    char from[LARGE_BUFFER_SIZE] = {'\0'} ;
    char to[LARGE_BUFFER_SIZE] = {'\0'} ;

    int pipe_fds[2] ; // the two FDs for reading/writing

    memset( from, 'h', LARGE_BUFFER_SIZE-1 ) ;
    memset( to, '3', LARGE_BUFFER_SIZE-1 ) ;

    // printf( "BEFORE: \n from: %s \n to: %s\n", from, to ) ;
    printf( "BEFORE\n" ) ;

    if( pipe(pipe_fds) )
    {
        perror( "Pipe ceration failed!" ) ;
        exit( EXIT_FAILURE ) ;
    }


    /*
     * the following doesn't work as LARGE_BUFFER_SIZE it much larger than the pipe
     * hence the write() doesn't finish
     *  
    ssize_t write_return = write( pipe_fds[1], &from, LARGE_BUFFER_SIZE ) ;
    printf( "Here!\n" ) ;
    assert( write_return == LARGE_BUFFER_SIZE ) ;

    ssize_t read_return = read( pipe_fds[0], &to, LARGE_BUFFER_SIZE ) ;
    */

    // What is the total amount to be written?
    int buffer_size = sizeof(from) ;

    size_t written = 0 ;
    while( buffer_size )
    {
        ssize_t write_return, read_return ;
        size_t write_amount = MIN( buffer_size, WRITE_CHUNK ) ; // MIN returns minimum of the two values

        // write the "write_amount" number of bytes
        write_return = write( pipe_fds[1], &from[written], write_amount ) ;
        if( write_return < 0 )
        {
            perror( "Error writing to pipe!" ) ;
            exit(EXIT_FAILURE) ;
        }

        // read the bytes out
        read_return = read( pipe_fds[0], &to[written], write_return ) ;
        assert( read_return == write_return ) ;

        // what's going on here?
        buffer_size -= write_return ;
        written += write_return ;
    }

    // printf( "AFTER: \n from: %s \n to: %s\n", from, to ) ;
    printf( "AFTER\n" ) ;

    assert( memcmp( from, to, sizeof(from) ) == 0 ) ; // look up memcmp()
    printf( "from and to are IDENTICAL!\n" ) ;

    printf( "\n" ) ;
    return 0 ;
}

