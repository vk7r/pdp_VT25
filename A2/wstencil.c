#include "stencil.h"
#include <assert.h>

int main(int argc, char **argv) {
	if (4 != argc) {
		printf("Usage: stencil input_file output_file number_of_applications\n");
		return 1;
	}
	char *input_name = argv[1];
	char *output_name = argv[2];
	int num_steps = atoi(argv[3]);
	
	MPI_Init(&argc, &argv);
	int rank, comm_sz, right_neighbour, left_neighbour;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	// Read input file
	double *input;
	int num_values;
	if (0 > (num_values = read_input(input_name, &input))) {
		return 2;
	}
	

	// Stencil values & decleractions
	double h = 2.0*PI/num_values;
	const int STENCIL_WIDTH = 5;
	const int EXTENT = STENCIL_WIDTH/2;
	const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};
	right_neighbour = (rank + 1) > comm_sz - 1 ? 0 : rank + 1;
	left_neighbour = (rank - 1) < 0 ? comm_sz - 1 : rank - 1;
	int slice = num_values / comm_sz;
	
	double *stencil_array = malloc(slice * sizeof(double));
	double *buf_r = malloc(EXTENT * sizeof(double));
	double *buf_l = malloc(EXTENT * sizeof(double));

	double *output;
	double *local_output;
	local_output = malloc(slice * sizeof(double));
	if (local_output == NULL) {
		perror("Couldn't allocate memory for local_output");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	MPI_Scatter(input, slice, MPI_DOUBLE, stencil_array, slice, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Start timer
	


	// Allocate data for result
	if(rank == 0){
		if (NULL == (output = malloc(num_values * sizeof(double)))) {
			perror("Couldn't allocate memory for output");
			return 2;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();
	
	// Repeatedly apply stencil
	//printf("num_steps %d\n", num_steps);
	for (int s=0; s<num_steps; s++) {
		MPI_Sendrecv(&stencil_array[0], EXTENT, MPI_DOUBLE, left_neighbour, 0, //send to the left & recive from right
			buf_r, EXTENT, MPI_DOUBLE, right_neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	
		MPI_Sendrecv(&stencil_array[slice - EXTENT], EXTENT, MPI_DOUBLE, right_neighbour, 0,
			buf_l, EXTENT, MPI_DOUBLE, left_neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		assert((sizeof(buf_l)/sizeof(double)) != 0);
		assert((sizeof(buf_r)/sizeof(double)) != 0);
		
		for (int i=0; i<slice; i++) {
			double result = 0.0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = i - EXTENT + j;
				if (index < 0) {
					result += STENCIL[j] * buf_l[EXTENT + index];
				}
				else if (index >= slice) {
					result += STENCIL[j] * buf_r[index - slice];
				}
				else {
					result += STENCIL[j] * stencil_array[index];
				}
			}
			local_output[i] = result;
		}
		if (s < num_steps - 1) {
			double *tmp = stencil_array;
			stencil_array = local_output;
			local_output = tmp;
		}
	}
	
	double *final_output = local_output;
	
	// Stop timer
	MPI_Barrier(MPI_COMM_WORLD);
	double my_execution_time = MPI_Wtime() - start;
	
	MPI_Gather(final_output, slice, MPI_DOUBLE, output, slice, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	double max_time;
	MPI_Reduce(&my_execution_time, &max_time, 1, MPI_DOUBLE,
           MPI_MAX, 0, MPI_COMM_WORLD);
	// Write result
	//printf("Time taken: %f seconds\n", my_execution_time);
	if (rank == 0){
		printf("Time taken: %f seconds\n", max_time);
		#ifdef PRODUCE_OUTPUT_FILE
			if (0 != write_output(output_name, output, num_values)) {
		
				return 2;
			}
		#endif
	}

	// Clean up
	if(rank == 0){
		free(output);
	}
	//printf("Succsess from, %d\n", rank);
	free(stencil_array);
	free(buf_r);
	free(buf_l);
	free(local_output);
	MPI_Finalize();
	return 0;
}


int read_input(const char *file_name, double **values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "r"))) {
		perror("Couldn't open input file");
		return -1;
	}
	int num_values;
	if (EOF == fscanf(file, "%d", &num_values)) {
		perror("Couldn't read element count from input file");
		return -1;
	}
	if (NULL == (*values = malloc(num_values * sizeof(double)))) {
		perror("Couldn't allocate memory for input");
		return -1;
	}
	for (int i=0; i<num_values; i++) {
		if (EOF == fscanf(file, "%lf", &((*values)[i]))) {
			perror("Couldn't read elements from input file");
			return -1;
		}
	}
	if (0 != fclose(file)){
		perror("Warning: couldn't close input file");
	}
	return num_values;
}


int write_output(char *file_name, const double *output, int num_values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%.4f ", output[i])) {
			perror("Couldn't write to output file");
		}
	}
	if (0 > fprintf(file, "\n")) {
		perror("Couldn't write to output file");
	}
	if (0 != fclose(file)) {
		perror("Warning: couldn't close output file");
	}
	return 0;
}
