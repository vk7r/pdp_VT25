#include "stencil.h"

int main(int argc, char **argv)
{
	if (4 != argc)
	{
		printf("Usage: stencil input_file output_file number_of_applications\n");
		return 1;
	}
	char *input_name = argv[1];
	char *output_name = argv[2];
	int num_steps = atoi(argv[3]);

	double *input;
	double *output;
	double *local_input;
	double *local_output;
	int num_values;
	int rank, num_proc;
	double start;

	// Read input file
	if (0 > (num_values = read_input(input_name, &input)))
	{
		return 2;
	}
	// Allocate data for result
	if (NULL == (output = malloc(num_values * sizeof(double))))
	{
		perror("Couldn't allocate memory for output");
		return 2;
	}

	// Stencil values
	double h = 2.0 * PI / num_values;
	const int STENCIL_WIDTH = 5;
	const int EXTENT = STENCIL_WIDTH / 2;
	const double STENCIL[] = {1.0 / (12 * h), -8.0 / (12 * h), 0.0, 8.0 / (12 * h), -1.0 / (12 * h)};

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int chunk_size = num_values / num_proc;
	// Allocate data for local input
	local_input = malloc(chunk_size * sizeof(double));
	local_output = malloc(chunk_size * sizeof(double));

	MPI_Barrier(MPI_COMM_WORLD);
	// Start timer
	start = MPI_Wtime();

	//	printf("number values %d\n", num_values);
	MPI_Scatter(input, chunk_size, MPI_DOUBLE, local_input, chunk_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//	printf("Rank %d received %d elements\n", rank, chunk_size);
	//	printf("Rank %d received %f\n", rank, local_input[0]);
	// Repeatedly apply stencil

	int local_num_values = chunk_size;
	for (int s = 0; s < num_steps; s++)
	{
		double left_gray[2] = {local_input[0], local_input[1]};
		double right_gray[2] = {local_input[chunk_size - 2], local_input[chunk_size - 1]};

		double borrowed_left_gray[2];
		double borrowed_right_gray[2];

		// Send to left, receive from right
		MPI_Sendrecv(
			left_gray, 2, MPI_DOUBLE, (rank - 1 + num_proc) % num_proc, 0, // send to left neighbor
			borrowed_right_gray, 2, MPI_DOUBLE, (rank + 1) % num_proc, 0,  // receive from right neighbor
			MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Send to right, receive from left
		MPI_Sendrecv(
			right_gray, 2, MPI_DOUBLE, (rank + 1) % num_proc, 1,					// send to right neighbor
			borrowed_left_gray, 2, MPI_DOUBLE, (rank - 1 + num_proc) % num_proc, 1, // receive from left neighbor
			MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		//	printf("Rank %d sent left [%f,%f]\n", rank, left_gray[0], left_gray[1]);
		//	printf("Rank %d sent right [%f,%f]\n", rank, right_gray[0], right_gray[1]);
		//	printf("Rank %d borrowed left [%f,%f]\n", rank, borrowed_left_gray[0], borrowed_left_gray[1]);
		//	printf("Rank %d borrowed right [%f,%f]\n", rank, borrowed_right_gray[0], borrowed_right_gray[1]);

		// Apply stencil //vänster
		for (int i = 0; i < EXTENT; i++)
		{
			double result = 0;
			for (int j = 0; j < STENCIL_WIDTH; j++)
			{
				int index = (i - EXTENT + j);
				if (index < 0)
				{
					result += STENCIL[j] * borrowed_left_gray[(index + 2) % 2];
				}
				else
				{
					result += STENCIL[j] * local_input[index];
				}
			}
			local_output[i] = result;
		}

		// Apply stencil //mitten
		for (int i = EXTENT; i < local_num_values - EXTENT; i++)
		{
			double result = 0;
			for (int j = 0; j < STENCIL_WIDTH; j++)
			{
				int index = i - EXTENT + j;
				result += STENCIL[j] * local_input[index];
			}
			local_output[i] = result;
		}

		for (int i=0; i<chunk_size; i++) {
			double result = 0.0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = i - EXTENT + j;
				if (index < 0) {
					result += STENCIL[j] * borrowed_left_gray[EXTENT + index];
				}
				else if (index >= chunk_size) {
					result += STENCIL[j] * borrowed_right_gray[index - chunk_size];
				}
				else {
					result += STENCIL[j] * local_input[index];
				}
			}
			local_output[i] = result;
		}

		// Apply stencil //höger
		for (int i = local_num_values - EXTENT; i < local_num_values; i++)
		{
			double result = 0;
			for (int j = 0; j < STENCIL_WIDTH; j++)
			{
				int index = (i - EXTENT + j);
				if (index >= local_num_values)
				{
					result += STENCIL[j] * borrowed_right_gray[((index - local_num_values) % 2)];
				}
				else
				{
					result += STENCIL[j] * local_input[index];
				}
				//	result += STENCIL[j] * local_input[index];
			}
			local_output[i] = result;
		}

		// Swap input and output
		if (s < num_steps - 1)
		{
			double *tmp = local_input;
			local_input = local_output;
			local_output = tmp;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	double execution_time = MPI_Wtime() - start;
	double max_time;
	MPI_Reduce(&execution_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Gather(local_output, chunk_size, MPI_DOUBLE, output, chunk_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	if (rank == 0)
		printf("Maximum execution time: %f\n", max_time);

#ifdef PRODUCE_OUTPUT_FILE
		if (write_output(output_name, output, num_values) != 0)
		{
			MPI_Abort(MPI_COMM_WORLD, 2);
		}
#endif

		free(input);
		free(output);
	}

	free(local_input);
	free(local_output);
	MPI_Finalize();

	return 0;
}

int read_input(const char *file_name, double **values)
{
	FILE *file;
	if (NULL == (file = fopen(file_name, "r")))
	{
		perror("Couldn't open input file");
		return -1;
	}
	int num_values;
	if (EOF == fscanf(file, "%d", &num_values))
	{
		perror("Couldn't read element count from input file");
		return -1;
	}
	if (NULL == (*values = malloc(num_values * sizeof(double))))
	{
		perror("Couldn't allocate memory for input");
		return -1;
	}
	for (int i = 0; i < num_values; i++)
	{
		if (EOF == fscanf(file, "%lf", &((*values)[i])))
		{
			perror("Couldn't read elements from input file");
			return -1;
		}
	}
	if (0 != fclose(file))
	{
		perror("Warning: couldn't close input file");
	}
	return num_values;
}

int write_output(char *file_name, const double *output, int num_values)
{
	FILE *file;
	if (NULL == (file = fopen(file_name, "w")))
	{
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++)
	{
		if (0 > fprintf(file, "%.4f ", output[i]))
		{
			perror("Couldn't write to output file");
		}
	}
	if (0 > fprintf(file, "\n"))
	{
		perror("Couldn't write to output file");
	}
	if (0 != fclose(file))
	{
		perror("Warning: couldn't close output file");
	}
	return 0;
}
