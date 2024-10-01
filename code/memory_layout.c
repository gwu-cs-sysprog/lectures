/*
 * CSC 2410 Code Sample 
 * memory layout and pointers in C
 * Fall 2024
 * (c) Sibin Mohan
 * 
 * Sept. 24, 2024.
 * 
 * 1145258561
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_SIZE 128
#define MAX_RECORDS 4

struct student{
    unsigned int _gwid ;
    // char _name[128] ;
    char* _name ;
} ;

// INTERFACE for creating student records
struct student* create_student_record( unsigned int gwid, char* name )
{
    struct student* new_student = (struct student*) malloc( sizeof( struct student ) ) ;
    new_student->_gwid = gwid ;
    new_student->_name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( new_student->_name, name ) ;

    return new_student ;
}

void delete_student_record( struct student* s )
{
    // we need to free the memory in s
    free(s->_name) ;
    free(s) ;
}


int main()
{
    struct student* new_student1 = create_student_record( 111111, "xxxxxx" ) ;

    /*
    new_student._gwid = 123456 ; 
    // new_student._name = "asdfgh" ;
    // *(new_student._name) = "asdfgh" ;
    new_student._name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( new_student._name, "asdfgh" ) ;
    */

    // struct student student_records[4] ;
    struct student* student_records[4] ;
    student_records[0] = create_student_record( 123456, "abc" ) ;
    student_records[1] = create_student_record( 234567, "def" ) ;
    student_records[2] = create_student_record( 345678, "ghi" ) ;
    student_records[3] = create_student_record( 456789, "jkl" ) ;


    student_records[0] = (struct student*) malloc( sizeof (struct student ) ) ;
    student_records[0]->_gwid = 123456 ;
    student_records[0]->_name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;  
    strcpy( student_records[0]->_name, "abc" ) ;

    student_records[1] = (struct student*) malloc( sizeof (struct student ) ) ;
    student_records[1]->_gwid = 234567 ;
    student_records[1]->_name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[1]->_name, "def" ) ;

    student_records[2] = (struct student*) malloc( sizeof (struct student ) ) ;
    student_records[2]->_gwid = 345678 ;
    student_records[2]->_name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[2]->_name, "ghi" ) ;

    student_records[3] = (struct student*) malloc( sizeof (struct student ) ) ;
    student_records[3]->_gwid = 456789 ;
    student_records[3]->_name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[3]->_name, "jkl" ) ;
    */

/*
    student_records[0]._gwid = 123456 ;
    student_records[0]._name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[0]._name, "abc" ) ;

    student_records[1]._gwid = 234567 ;
    student_records[1]._name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[1]._name, "def" ) ;

    student_records[2]._gwid = 345678 ;
    student_records[2]._name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[2]._name, "ghi" ) ;

    student_records[3]._gwid = 456789 ;
    student_records[3]._name = (char*) malloc( sizeof(char)*MAX_NAME_SIZE ) ;
    strcpy( student_records[3]._name, "jkl" ) ;
*/

    printf( "\nGWID \t\t NAME\n" ) ;
    printf( "------------------------\n" ) ;
    for( unsigned int i = 0 ; i < MAX_RECORDS ; ++i )
    {
        printf( "%d \t\t %s\n", 
                student_records[i]->_gwid, student_records[i]->_name ) ;
    }


    for(unsigned int i = 0 ; i < 4 ; ++i )
        delete_student_record( student_records[i] ) ;


    printf( "\n" ) ;
    return 0 ;
}