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

int main()
{
    pid_t child ;
    pid_t parent ;

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
            break ;

        // printf( "%d....\n",i ) ;
    }

    // CONCURRENCY
    
    int wait_status ;
    pid_t wait_result = wait( &wait_status ) ; // only parents can wait

    printf( ":::: AFTER LOOP ::::\n" ) ;
    child ? printf( "----- parent ------ \n" ) : printf( "------ child ------" ) ;

    printf( "getpid() = %d \
            parent = %d \
            child = %d \n",
            getpid(), getppid(), child ) ;
    printf( "-------------------------\n" ) ;



    printf( "\n" ) ;
    return 0 ;
}