#include <stdio.h>
#include <stdlib.h>
#define SHIFT 16384

int int_to_fixed[100];

void fixed_init(void);
int convert_int_to_fixed(int n);
int convert_fixed_to_int_rd(int x);
int convert_fixed_to_int_near(int x);
int add_fixed_and_fixed(int x, int y);
int sub_fixed_from_fixed(int x, int y);
int add_fixed_and_int(int x, int n);
int sub_int_from_fixed(int x, int n);
int multiply_fixed_by_fixed(int x, int y);
int multiply_fixed_by_int(int x, int n);
int divide_fixed_by_fixed(int x, int y);
int divide_fixed_by_int(int x, int n);
