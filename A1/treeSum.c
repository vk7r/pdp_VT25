#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 9

int array[ARRAY_SIZE] = {2, 3, 4, 5, 6, 7, 8, 9, 10};

int main(int argc, char **argv)
{
    int rank, size;
    int sum = 0;
    int received;
    int num;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // printf("Size is %d\n", size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // int num = rank; //+ rand() % 100;
    int sizeOfArray = sizeof(array) / sizeof(array[0]);
    int block = sizeOfArray / size;
    int start = rank * block;
    int end = (sizeOfArray % size != 0 && rank == (size - 1)) ? (start + block + (sizeOfArray % size) - 1) : start + block - 1;
    printf("Rank %d starts with %d and ends with %d\n", rank, start, end);

    for (int j = start; j <= end; j++)
    {
        sum += array[j];
    }
    // alternative way to hand out the array
    /*  for (int i = 0; i < sizeOfArray; i++)
        {
            if (i % size == rank)
            {
                sum += array[i];
            }
        }
    */

    num = sum;
    // printf("Rank %d STARTS WITH %d\n", rank, sum);
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
    }

    MPI_Finalize();

    return 0;
}
