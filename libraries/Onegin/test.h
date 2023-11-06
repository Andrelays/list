#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include<stdio.h>

void print_buffer(const char *buffer, size_t size_file);
void print_string(const char *buffer);
void print_partition(const int *left_border, const int *right_border, int *pivot, int *array, size_t number_elements);

#endif //TEST_H_INCLUDED