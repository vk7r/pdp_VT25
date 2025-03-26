#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char **argv)
{
	int rank, num_processes;
	int local_number;
	int local_sum, global_sum;

	// Initialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

	// Seed random number generator differently for each process
	srand(time(NULL) * (rank + 1));

	// Each process generates its own random local number
	local_number = rand() % 100; // Random number between 0-99
	local_sum = local_number;

	printf("Process %d: Local number = %d\n", rank, local_number);

	// Tree-structured reduction
	for (int step = 1; step < num_processes; step *= 2)
	{
		// Calculate partner process
		int partner;
		if (rank % (2 * step) == 0)
		{
			// Receiving process
			partner = rank + step;
			if (partner < num_processes)
			{
				int received_sum;
				MPI_Recv(&received_sum, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				local_sum += received_sum;
			}
		}
		else
		{
			printf("Process %d: step = %d\n", rank, step);
			// Sending process
			partner = rank - step;
			MPI_Send(&local_sum, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
			break; // Sending process stops after sending
		}
	}

	// Root (process 0) prints global sum
	if (rank == 0)
	{
		global_sum = local_sum;
		printf("\nGlobal Sum: %d\n", global_sum);
	}

	// Finalize MPI
	MPI_Finalize();

	return 0;
}