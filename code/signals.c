/* CSC 2410 Code Sample 
 * signals
 * Fall 2024
 * (c) Sibin Mohan
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>  // sigaction and SIG
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

typedef void (*signal_handler_function)( int signal_number, 
                                        siginfo_t* info, 
                                        void* context ) ;

// signal handler
void seg_fault_handler( int signal_number, siginfo_t* info, void* context )
{
    printf( "My downfall is the forbidden address at %p\n", info->si_addr ) ;

    exit( EXIT_FAILURE ) ;

    return ;

}

void my_timer_handler( int signal_number, siginfo_t* info,
                                            void* context )
{
    // Handler for SIGALRM

    printf( "Inside Alarm Handler!\n" ) ;

    return ;
}


void setup_signal( int signal_number, signal_handler_function func, int sh_flags )
{
   sigset_t masked ;
   struct sigaction siginfo ;
   sigemptyset( &masked ) ;
   sigaddset( &masked, signal_number ) ;
   siginfo = (struct sigaction){
        .sa_sigaction = func,
        .sa_mask = masked, 
        .sa_flags = sh_flags
   } ;

   if( sigaction( signal_number, &siginfo, NULL ) == -1 )
   {
        perror( "signal setup failed" ) ;
        exit(EXIT_FAILURE) ;
   }    
}

int main()
{
    /*
    int* a = (int*) NULL ;
    // *a = 100 ;

    int b = 100/0 ;
    */

   /*
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
   */
    setup_signal( SIGSEGV, seg_fault_handler, SA_RESTART | SA_SIGINFO ) ;
    setup_signal( SIGALRM, my_timer_handler, SA_SIGINFO  | SA_RESTART ) ;

    pid_t pid ;
    if( (pid = fork()) == 0 )
    {
        // Child process
        pause() ;
        exit( EXIT_SUCCESS ) ;
    }

    alarm(1) ;

    while(1)
    {
        pid_t ret = wait(NULL) ;

        if( ret == -1 )
        {
            // Problems with child
            if( errno == EINTR )
            {
                // Child process exited due to signal
                printf( "Child process exited due to signal\n" ) ;
                kill( pid, SIGTERM ) ;

                return -1 ;
            }
            else if( errno = ECHILD )
            {
                // should never come here!
                printf( "Child process exited cleanly\n" ) ;
                return 0 ;
            }
        }
    }

    // int* a = (int*) NULL ;
    // *a = 100 ;

    printf( "AFTER\n" ) ;


    printf( "\n" ) ;
    return 0 ;
}