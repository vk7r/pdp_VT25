#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// #define ARRAY_SIZE 4194304

/*
static double get_wall_seconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double seconds = tv.tv_sec + (double)tv.tv_usec / 1000000;
    return seconds;
}
    */

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
    // printf("Size is %d\n", size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int start, end;

    if (rank == 0)
    {
        int *array = (int *)malloc(ARRAY_SIZE * sizeof(int));
        srand(42);
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            // array[i] = 1;
            array[i] = rand() % 100;
        }
        int block = ARRAY_SIZE / size;
        for (int i = 0; i < size; i++)
        {
            start = i * block;
            end = (ARRAY_SIZE % size != 0 && i == (size - 1)) ? (start + block + (ARRAY_SIZE % size) - 1) : start + block - 1;
            
            if (i == 0)
            {
                for (int j = start; j <= end; j++)
                {
                    sum += array[j];
                }
            }
            else
            {
                MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&array[start], end - start + 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        free(array);
    }
    else
    {
        MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&end, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        int blockSize = end - start + 1;
        int *subArray = (int *)malloc(blockSize * sizeof(int));
        MPI_Recv(subArray, blockSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int j = 0; j < blockSize; j++)
        {
            sum += subArray[j];
        }
        
        free(subArray);
    }
    
    num = sum;
    // printf("Rank %d STARTS WITH %d\n", rank, sum);
    startTime = MPI_Wtime(); // get_wall_seconds();
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
                //       printf("Rank %d received %d from Rank %d\n", rank, sum, partner);
            }
        }
        else // will never be rank 0, send to partner depending on the div
        {
            partner = rank - div;
            //      printf("I: %d WANT TO SEND %d\n", rank, num);
            MPI_Send(&num, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            break;
        }
    }

    if (rank == 0)
    {
        sum = num;
        printf("Sum is %d\n", sum);
        printf("The summation took %f wall seconds.\n", MPI_Wtime() - startTime);
    }

    MPI_Finalize();

    return 0;
}
