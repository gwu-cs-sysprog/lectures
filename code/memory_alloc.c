/*
 * CSC 2410 Code Sample 
 * memory allocation in C
 * Fall 2024
 * (c) Sibin Mohan
 * 
 * Sept. 17, 2024.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define HAIKU_SIZE 128
#define POEM_SIZE 1028
// #define A B

int main()
{
    // int* temp = malloc( 239847 ) ;
    // int* p_int = (int*) malloc( sizeof(int) * 100 ) ;

    char haiku[] = "'You Laughed While I Slept'\n\
- by Bertram Dobell\n\
\n\
You laughed while I wept,\n\
Yet my tears and your laughter\n\
Had only one source." ;

    // char* new_haiku = (char*) malloc( sizeof(haiku) ) ;
    char* new_haiku = (char*) malloc( HAIKU_SIZE ) ;
    assert(new_haiku) ;

    // SHALLOW COPY
    // new_haiku = haiku ;


    // DEEP COPY
    /*
    unsigned int i ;
    for( i = 0 ; i < strlen(haiku) ; ++i )
        new_haiku[i] = haiku[i] ;
    */

   strcpy( new_haiku, haiku ) ;

    // printf( "haiku = %s\n\n", haiku ) ;

    haiku[0] = '#' ;
    // 1[haiku] = '*' ; 

    printf( "haiku = %s\n\n", haiku ) ;
    printf( "new_haiku = %s\n\n", new_haiku ) ;

    char twain_poem[] = "'These Annual Bills'\n\
- Mark Twain\n\
\n\
These annual bills! these annual bills!\n\ 
How many a song their discord trills \n\
Of 'truck' consumed, enjoyed, forgot,\n\
Since I was skinned by last year's lot!\n\
Those joyous beans are passed away;\n\
\n\
Those onions blithe, O where are they?\n\
Once loved, lost, mourned-now vexing ILLS\n\
Your shades troop back in annual bills! \n\
\n\
And so 'twill be when I'm aground \n\
These yearly duns will still go round, \n\
While other bards, with frantic quills,\n\
\n\
Shall damn and damn these annual bills!" ;

    new_haiku = (char*) realloc( new_haiku, POEM_SIZE ) ;
    assert(new_haiku) ;

    // check if pointer is valid
    /*
    // if( !new_haiku )
    if( new_haiku == NULL )
    {
        printf( "ERROR!\n" ) ;
        return -1 ;
    }
    */
   // assert(0) ;

    strcpy( new_haiku, twain_poem ) ;

    printf( "new_haiku = %s\n\n", new_haiku ) ;

    char* temp = new_haiku ;

    // MEMORY LEAK
    free(new_haiku) ;

    // DANGLING POINTER
    temp[1000] = '!' ;
    printf( "temp = %s\n\n", temp ) ;

    printf( "\n") ;
    return 0 ;
}