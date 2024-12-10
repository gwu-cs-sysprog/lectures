/* CSC 2410 Code Sample 
 * files and file handling
 * Fall 2024
 * (c) Sibin Mohan
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>

#define TWEET_LEN 280

int main()
{
    char tweet[TWEET_LEN] ;

    int fd = open( "./daffodils.txt", O_RDONLY ) ;
    if( fd == -1 )
    {
        perror( "File daffodils.txt failed to open" ) ;
        exit(EXIT_FAILURE) ;
    }

    struct stat file_info ;
    int ret = fstat( fd, &file_info ) ;
    // int ret = stat( "./daffodils.txt", &file_info ) ;
    assert( ret >= 0 ) ;
    printf( "Number of characters in file = %ld\n", file_info.st_size ) ;

    // Read TWEET_LEN number or characters from the file
    int current_read = 0 ;
    int i = 1 ; 
    while( current_read < file_info.st_size )
    {
        ret = read( fd, tweet, TWEET_LEN ) ;
        assert( ret == TWEET_LEN ) ;
        printf( "Tweet Number %d\n %s \n\n", i++, tweet ) ;

        lseek( fd, 0, SEEK_SET ) ;

        current_read += ret ;
    }

    close(fd) ;

    printf( "\n" ) ;
    return 0 ;
}