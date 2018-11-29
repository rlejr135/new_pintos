#include "fixed_point.h"

////////////////////////////////////////////////////////////////////////
//
// calculate fixed point
//
//	n is integer
//	x, y is fixed point
//  SHIFT is used to convert fixed point to integer or integer to fixed point
//
////////////////////////////////////////////////////////////////////////
int convert_int_to_fixed(int n){
	return n * SHIFT;
}


int convert_fixed_to_int_rd(int x){
	return x / SHIFT;
}

int convert_fixed_to_int_near(int x){
	if (x >= 0){
		return (x + SHIFT / 2) / SHIFT;
	}
	else{
		return (x - SHIFT / 2) / SHIFT;
	}
}

int add_fixed_and_fixed(int x, int y){
	return x + y;
}

int sub_fixed_from_fixed(int x, int y){
	return x - y;
}

int add_fixed_and_int(int x, int n){
	return x + n * SHIFT;
}

int sub_int_from_fixed(int x, int n){
	return x - n * SHIFT;
}

int multiply_fixed_by_fixed(int x, int y){
	return ((int64_t) x) * y / SHIFT;
}

int multiply_fixed_by_int(int x, int n){
	return x * n;
}

int divide_fixed_by_fixed(int x, int y){
	return ((int64_t) x) * SHIFT / y;
}

int divide_fixed_by_int(int x, int n){
	return x / n;
}
