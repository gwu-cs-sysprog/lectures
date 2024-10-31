/* CSC 2410 Code Sample 
 * processes | wait()
 * Fall 2024
 * (c) Sibin Mohan
 */


#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h> 

#define NUM_CHILDREN 5

void cleanup( int status, void* args )
{
    // status -- same STATUS from exit/wait
    // args -- any uder defined argument (of any type)

    free( args ) ;
    printf( "Done with free in cleanup()\n" ) ;
}

int main()
{
    pid_t child ;
    pid_t parent ;

    int* some_memory = (int*)malloc( 2783654 ) ;
    on_exit( cleanup, some_memory ) ;

    printf( "-------------------------\n" ) ;
    printf( "::: BEFORE LOOP ::::    \
            getpid() = %d \
            parent = %d \
            child = %d \n",
            getpid(), getppid(), child) ;
    printf( "-------------------------\n" ) ;

    for( unsigned int i = 0 ; i < NUM_CHILDREN ; ++i )
    {
        child = fork() ;

        if( !child )
        {
            if( i % 2 )
                exit(i) ;
            else 
                return i ;
        }

        // printf( "%d....\n",i ) ;
    }

    int wait_status ;
    pid_t wait_result = wait( &wait_status ) ; // only parents can wait

    printf( ":::: AFTER LOOP ::::\n" ) ;
    // child ? printf( "----- parent ------ \n" ) : printf( "------ child ------" ) ;
    while( (child = wait(&wait_status) ) != -1 )
    {
        // wait on ANY chld
        // ANY child exits
        // "child" getd PID of exited child
        // enter WHILE loop

        if( WIFEXITED(wait_status) )
            printf( "Child with PID: %d exited with status: %d\n", child, WEXITSTATUS(wait_status) ) ;
    }

    printf( "\n" ) ;
    return 0 ;
    // return EXIT_SUCCESS ;
    // exit(EXIT_SUCCESS) ;
}