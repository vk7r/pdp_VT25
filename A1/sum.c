#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
	if (2 != argc) {
		printf("Usage: sum number_of_summands\n");
		return 1;
	}
	int num_steps = atoi(argv[1]);


	printf("Parallel Summation\n");

	return 0;
}
