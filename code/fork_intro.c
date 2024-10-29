/* CSC 2410 Code Sample 
 * processes | fork()
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <stdio.h>
#include <unistd.h> // fork()
#include <sys/types.h> // pid_t

int main()
{
    printf( "BEFORE Calling fork() \n" ) ;
    fflush(stdout) ;

    pid_t new_pid = fork() ;

    if( !new_pid ) // new_pid == 0
    {
        printf( "Inside CHILD\n" ) ;
    } 
    else
        printf( "Inside PARENT\n" ) ;

    printf( "AFTER Calling fork() \t PID = %d ", new_pid ) ;
    fflush(stdout) ;

    printf( "\n" ) ;
    return 0 ;
}