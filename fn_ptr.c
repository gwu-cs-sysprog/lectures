#include <stdio.h>

typedef int (*comparator_function_pointer)(void* l, void* r);
typedef void (*swap_function_pointer)(void* l, void* r);

void generic_bubble_sort(void* array, int array_size,
                         int element_size,
                         comparator_function_pointer is_greater,
                         swap_function_pointer swap) {
    if (array_size < 2)
        return;

    unsigned char* base = array;

    for (int i = 0; i < array_size - 1; ++i)
        for (int j = 0; j < array_size - 1 - i; ++j)
            if (is_greater(base + (j * element_size), base + ((j + 1) * element_size)))
                swap(base + (j * element_size), base + ((j + 1) * element_size));
}

// Compare and swap functions for integers
int is_greater_than_int(void* l, void* r) {
    // Convert the void pointers before dereferencing them.
    int* left = l;
    int* right = r;

    if (*left > *right)
        return 1;
    else
        return 0;

    // Equivalent: return *left > *right;
}

void swap_int(void* l, void* r) {
    int* left = l;
    int* right = r;
    int temp = *left;

    *left = *right;
    *right = temp;
}

// Compare and swap functions for doubles
int is_greater_than_double(void* l, void* r) {
    double* left = l;
    double* right = r;

    if (*left > *right)
        return 1;
    else
        return 0;
}

void swap_double(void* l, void* r) {
    double* left = l;
    double* right = r;
    double temp = *left;

    *left = *right;
    *right = temp;
}

int main(void) {
    int my_array_int[] = { 2341, 8632, 3, 2344, 747645 };
    int array_size = 5;

    // Sort integers using the integer comparator and swap functions.
    generic_bubble_sort(my_array_int, array_size,
                        sizeof(int), /*element size*/
                        is_greater_than_int,
                        swap_int);

    // Sort doubles using the double comparator and swap functions.
    double my_double_array[] = { 1.0, 9485.2, 34.567, 9383.243, 44.1 };
    generic_bubble_sort(my_double_array, array_size,
                        sizeof(double), /*element size*/
                        is_greater_than_double,
                        swap_double);

    printf("Sorted integers:");
    for (int i = 0; i < array_size; ++i)
        printf(" %d", my_array_int[i]);
    printf("\n");

    printf("Sorted doubles:");
    for (int i = 0; i < array_size; ++i)
        printf(" %.10g", my_double_array[i]);
    printf("\n");

    return 0;
}
