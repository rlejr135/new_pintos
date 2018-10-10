#include <stdio.h>
#include "syscall.h"
#include "../lib/user/syscall.h"

int main(int argc, char *argv[]){
	argv[0] = atoi(argv[0]);
	argv[1] = atoi(argv[1]);
	argv[2] = atoi(argv[2]);
	argv[3] = atoi(argv[3]);
	argv[4] = atoi(argv[4]);

	printf("%d %d\n", fibonacci(argv[1]), sum_of_four_integers(argv[1], argv[2], argv[3], argv[4]));

	return EXIT_SUCCESS;
}
