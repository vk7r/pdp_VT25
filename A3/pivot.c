#include <stdio.h>
#include <mpi.h>

#include "pivot.h"

int get_median(int *elements, int n)
{
    // If even, return the average of the two middle elements
    // if odd, return the middle element
    if (n % 2 == 0)
    {
        return (elements[n / 2 - 1] + elements[n / 2]) / 2;
    }
    else
    {
        return elements[n / 2];
    }
}

int get_larger_index(int *elements, int n, int val)
{
    for (int i = 0; i < n; i++)
    {
        if (elements[i] > val)
        {
            return i;
        }
    }
    return n; // If all elements are smaller or equal to val
}

int select_pivot(int pivot_strategy, int *elements, int n, MPI_Comm communicator)
{
    if (pivot_strategy == 1)
    {
        return select_pivot_median_root(elements, n, communicator);
    }
    else if (pivot_strategy == 2)
    {
        return -2; // select_pivot_mean_median(elements, n, communicator);
    }
    else if (pivot_strategy == 3)
    {
        return -2; // select_pivot_median_median(elements, n, communicator);
    }
    else
    {
        fprintf(stderr, "Error: Invalid pivot strategy %d\n", pivot_strategy);
        MPI_Abort(communicator, -1);
    }
    return -1; // This line should never be reached
}

/**
 * Select a pivot element for parallel quick sort. Return the index of the first
 * element that is larger than the pivot. Note that this function assumes that
 * elements is sorted!
 * @param pivot_strategy 0=>smallest on root (Not recommended!) 1=>median on root 2=>mean of medians 3=>median of medians
 * @param elements Elements stored by the current process (sorted!)
 * @param n Length of elements
 * @param communicator Communicator for processes in current group
 * @return The index of the first element after pivot
 */
int select_pivot_median_root(int *elements, int n, MPI_Comm communicator)
{
    int rank;
    MPI_Comm_rank(communicator, &rank);
    int pivot = 0;
    if (rank == 0)
    {
        pivot = get_median(elements, n);
    }
    // Broadcast the pivot to all processes
    MPI_Bcast(&pivot, 1, MPI_INT, 0, communicator);
    int index = get_larger_index(elements, n, pivot);
    if (rank == 0)
    {
        printf("Pivot: %d\n", pivot);
    }
    return index;
}