/* CSC 2410 Code Sample 
 * signals
 * Fall 2024
 * (c) Sibin Mohan
 */


#include <stdio.h>
#include <signal.h> // sigaction and SIG
#include <errno.h>
#include <string.h>
#include <stdlib.h>

// signal handler
void seg_fault_handler( int signal_number, siginfo_t* info, void* context )
{
    printf( "My downfall is the forbidden address at %p\n", info->si_addr ) ;

    // exit( EXIT_FAILURE ) ;

    return ;

} 

int main()
{
    /*
    int* a = (int*) NULL ;
    // *a = 100 ;

    int b = 100/0 ;
    */

   sigset_t masked ;
   struct sigaction siginfo ;
   sigemptyset( &masked ) ;
   sigaddset( &masked, SIGSEGV ) ;
   siginfo = (struct sigaction){
        .sa_sigaction = seg_fault_handler,
        .sa_mask = masked, 
        .sa_flags = SA_RESTART | SA_SIGINFO
   } ;

   if( sigaction( SIGSEGV, &siginfo, NULL ) == -1 )
   {
        perror( "sigaction failed" ) ;
        exit(EXIT_FAILURE) ;
   } 

    int* a = (int*) NULL ;
    *a = 100 ;

    printf( "AFTER\n" ) ;


    printf( "\n" ) ;
    return 0 ;
}