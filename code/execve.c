/* CSC 2410 Code Sample 
 * processes | exec ()
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <stdio.h>
#include <unistd.h>  // exec*, fork()
#include <sys/types.h> // pid_t
#include <sys/wait.h> // wait
#include <stdlib.h> // exit

// VARIADIC FUNCTION

// argc -- number of command line args
// argv[] -- array of command line args, each a string
int main( int argc, char* argv[] )
{
    // printf( "%d", 10 ) ;
    /*
    if( argc )
    {
        // positive number of args
        // FIRST arg is the program name

        printf( "Command Line arguments received: " ) ;
        for( unsigned int i = 0 ; i < argc ; ++i )
            printf( "%s \n", argv[i] ) ;
    }
    */

    // char* program = "/bin/ls" ;
    char* program = "./child" ;
    char* arg1 = "ljhkdfjh" ;
    char* arg2 = "8798798" ;

    printf( "BEFORE EXEC!\n" ) ;

    pid_t child = fork() ;

    if( !child )
    {
        int ret = execl( program, "banana", arg1, arg2, NULL ) ;
    }

    int status ;
    pid_t wait_pid = wait(&status) ;

    if( WIFEXITED(status) )
    {
        // child is done
        printf( "--- Parent ---: Child process %d exited with status %d\n",
            child, WEXITSTATUS(status) ) ;

        printf( "AFTER EXEC!\n" ) ;
    }

    printf( "\n" ) ;
    return 0 ;
}