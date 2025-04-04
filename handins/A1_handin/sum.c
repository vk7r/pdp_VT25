#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int rank, size;
    int sum = 0;
    int received;
    int num;
    double startTime;

    if (argc != 2)
    {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    int ARRAY_SIZE = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int block_size = ARRAY_SIZE / size;
    int remainder = ARRAY_SIZE % size;

    int my_block_size = (rank < remainder) ? block_size + 1 : block_size;

    int block_starts[size];
    int send_counts[size];

    if (rank == 0)
    {

        int nxt_block = 0;
        for (int i = 0; i < size; i++)
        {
            send_counts[i] = (i < remainder) ? block_size + 1 : block_size;
            block_starts[i] = nxt_block;
            nxt_block += send_counts[i];
        }
    }

    int *array = NULL;
    if (rank == 0)
    {
        array = (int *)malloc(ARRAY_SIZE * sizeof(int));
        ;
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            array[i] = rand() % 100;
        }
    }

    int *local_array = (int *)malloc(my_block_size * sizeof(int));

    MPI_Scatterv(array, send_counts, block_starts, MPI_INT,
                 local_array, my_block_size, MPI_INT,
                 0, MPI_COMM_WORLD);
    startTime = MPI_Wtime();
    for (int i = 0; i < my_block_size; i++)
    {
        sum += local_array[i];
    }

    num = sum;

    for (int div = 1; div < size; div *= 2)
    {
        int partner;

        if (rank % (div * 2) == 0)
        {
            partner = rank + div;
            if (partner < size) // Check if the rank is not the last one
            {
                MPI_Recv(&received, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                num += received;
            }
        }
        else // will never be rank 0, send to partner depending on the div
        {
            partner = rank - div;
            MPI_Send(&num, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            break;
        }
    }
    if (rank == 0)
    {
        sum = num;
        printf("Sum is %d\n", sum);
        printf("The summation took %f wall seconds.\n", MPI_Wtime() - startTime);

        // Free allocated memory
        free(array);
    }

    free(local_array);
    MPI_Finalize();

    return 0;
}