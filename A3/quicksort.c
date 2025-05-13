#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "pivot.h"
#include "quicksort.h"
#include <string.h>

int read_input(char *file_name, int **elements)
{
    FILE *file;
    if (NULL == (file = fopen(file_name, "r")))
    {
        perror("Couldn't open input file");
        return -1;
    }
    int num_elements;
    if (EOF == fscanf(file, "%d", &num_elements))
    {
        perror("Couldn't read element count from input file");
        return -1;
    }
    if (NULL == (*elements = malloc(num_elements * sizeof(int))))
    {
        perror("Couldn't allocate memory for input");
        return -1;
    }
    for (int i = 0; i < num_elements; i++)
    {
        if (EOF == fscanf(file, "%d", &((*elements)[i])))
        {
            perror("Couldn't read elements from input file");
            return -1;
        }
    }
    if (0 != fclose(file))
    {
        perror("Warning: couldn't close input file");
    }
    return num_elements;
}

int check_and_print(int *elements, int n, char *file_name)
{
    FILE *file = fopen(file_name, "w");
    if (!file)
    {
        perror("Couldn't open output file");
        return -2;
    }

    for (int i = 0; i < n; i++)
    {
        fprintf(file, "%d ", elements[i]);
    }
    fprintf(file, "\n");

    fclose(file);
    return 0;
}

int distribute_from_root(int *all_elements, int n, int **my_elements)
{
    int rank, num_proc;

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int chunk_size = n / num_proc;
    int remainder = n % num_proc;
    int block_starts[num_proc];
    int send_counts[num_proc];

    int nxt_block = 0;
    for (int i = 0; i < num_proc; i++)
    {
        send_counts[i] = (i < remainder) ? chunk_size + 1 : chunk_size;
        block_starts[i] = nxt_block;
        nxt_block += send_counts[i];
    }

    int my_count = send_counts[rank];
    *my_elements = malloc(n * sizeof(int));

    MPI_Scatterv(all_elements, send_counts, block_starts, MPI_INT,
                 *my_elements, my_count, MPI_INT,
                 0, MPI_COMM_WORLD);
    return my_count;
}

void merge_ascending(int *v1, int n1, int *v2, int n2, int *result)
{
    int i = 0, j = 0, k = 0;
    while (i < n1 && j < n2)
    {
        if (v1[i] < v2[j])
        {
            result[k++] = v1[i++];
        }
        else
        {
            result[k++] = v2[j++];
        }
    }
    while (i < n1)
    {
        result[k++] = v1[i++];
    }
    while (j < n2)
    {
        result[k++] = v2[j++];
    }
}

void gather_on_root(int *all_elements, int *my_elements, int local_n)
{
    int rank, num_proc;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int block_starts[num_proc];
    int send_counts[num_proc];
    int nxt_block = 0;
    // Gather local_n values from each process
    MPI_Gather(&local_n, 1, MPI_INT, send_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    for (int i = 0; i < num_proc; i++)
    {
        block_starts[i] = nxt_block;
        nxt_block += send_counts[i];
    }

    MPI_Gatherv(my_elements, local_n, MPI_INT,
                all_elements, send_counts, block_starts, MPI_INT,
                0, MPI_COMM_WORLD);
}

int global_sort(int **elements, int n, MPI_Comm communicator, int pivot_strategy, int recursion_level)
{
    int larger_index = select_pivot(pivot_strategy, *elements, n, communicator);
    int size_set_small = larger_index;
    int size_set_large = n - larger_index;
    // int set_small[size_set_small];
    // int set_large[size_set_large];
    int *set_small = malloc(size_set_small * sizeof(int));
    int *set_large = malloc(size_set_large * sizeof(int));

    int size, rank;
    MPI_Group world_group;
    MPI_Comm_group(communicator, &world_group);
    MPI_Comm_size(communicator, &size);
    MPI_Comm_rank(communicator, &rank);
    MPI_Group small_group;
    MPI_Comm small_comm;
    MPI_Group big_group;
    MPI_Comm big_comm;
    int Recv_set_size = 0;

    int j = 0;
    for (int i = 0; i < n; i++)
    {
        if (i < larger_index)
        {
            set_small[i] = (*elements)[i];
        }
        else
        {
            set_large[j] = (*elements)[i];
            j++;
        }
    }

    // skapa nya comm grupper --> Small & Big  small_group = [ 0, 1, 2, 3 ] big_group = [ 4, 5, 6, 7 ],  small_group = [ 0, 1] big_group = [ 2, 3] small_group = [4,5 ] big_group = [ 6, 7 ]
    int middle = size / 2;
    int small_ranks[middle];
    int big_ranks[middle];

    for (int i = 0; i < size; i++)
    {
        if (i < middle)
        {
            small_ranks[i] = i;
        }
        else
        {
            big_ranks[i % middle] = i;
        }
    }

    MPI_Group_incl(world_group, middle, small_ranks, &small_group);
    MPI_Comm_create(communicator, small_group, &small_comm);

    // // Create big group and communicator
    MPI_Group_incl(world_group, middle, big_ranks, &big_group);
    MPI_Comm_create(communicator, big_group, &big_comm);

    int small_size, small_rank;
    int big_size, big_rank;

    if (rank < middle)
    {
        // SMALL GROUP
        MPI_Comm_size(small_comm, &small_size);
        MPI_Comm_rank(small_comm, &small_rank);

        // Send to a corresponding partner in the big group

        int partner = big_ranks[small_rank];
        // Send the size of the buffer to transfer // Receive the size of received buffer
        MPI_Sendrecv(&size_set_large, 1, MPI_INT, partner, 0,
                     &Recv_set_size, 1, MPI_INT, partner, 0,
                     communicator, MPI_STATUS_IGNORE);

        int *Recv_set = malloc(Recv_set_size * sizeof(int));

        // Swap set of numbers with "partner" process in other group.
        MPI_Sendrecv(set_large, size_set_large, MPI_INT, partner, 0,
                     Recv_set, Recv_set_size, MPI_INT, partner, 0,
                     communicator, MPI_STATUS_IGNORE);

        // Populate new list
        int new_list_size = size_set_small + Recv_set_size;
        merge_ascending(set_small, size_set_small, Recv_set, Recv_set_size, (*elements));

        // recursion if there are more than 1 process in the group
        if (small_size != 1)
        {
            int result = global_sort(elements, new_list_size, small_comm, pivot_strategy, recursion_level + 1);
            // Free communicators after recursion
            if (small_comm != MPI_COMM_NULL)
                MPI_Comm_free(&small_comm);
            if (big_comm != MPI_COMM_NULL)
                MPI_Comm_free(&big_comm);
            free(Recv_set);
            free(set_small);
            free(set_large);
            return result;
        }
        else
        {
            // Free communicators after recursion
            if (small_comm != MPI_COMM_NULL)
                MPI_Comm_free(&small_comm);
            if (big_comm != MPI_COMM_NULL)
                MPI_Comm_free(&big_comm);
            free(Recv_set);
            free(set_small);
            free(set_large);
            //
            return new_list_size;
        }
    }
    else
    {
        // BIG GROUP
        MPI_Comm_size(big_comm, &big_size);
        MPI_Comm_rank(big_comm, &big_rank);
        // Send to a corresponding partner in the small group
        int partner = small_ranks[big_rank];
        // Send the size of the buffer to transfer // Receive the size of received buffer
        MPI_Sendrecv(&size_set_small, 1, MPI_INT, partner, 0,
                     &Recv_set_size, 1, MPI_INT, partner, 0,
                     communicator, MPI_STATUS_IGNORE);

        int *Recv_set = malloc(Recv_set_size * sizeof(int));

        // Swap set of numbers with "partner" process in other group.
        MPI_Sendrecv(set_small, size_set_small, MPI_INT, partner, 0,
                     Recv_set, Recv_set_size, MPI_INT, partner, 0,
                     communicator, MPI_STATUS_IGNORE);
        // Populate new list
        int new_list_size = size_set_large + Recv_set_size;
        merge_ascending(set_large, size_set_large, Recv_set, Recv_set_size, (*elements));

        // recursion if there are more than 1 process in the group
        if (big_size != 1)
        {
            int result = global_sort(elements, new_list_size, big_comm, pivot_strategy, recursion_level + 1);

            // Free communicators after recursion
            if (small_comm != MPI_COMM_NULL)
                MPI_Comm_free(&small_comm);
            if (big_comm != MPI_COMM_NULL)
                MPI_Comm_free(&big_comm);
            free(Recv_set);
            free(set_small);
            free(set_large);

            return result;
        }
        else
        {
            // Free communicators after recursion
            if (small_comm != MPI_COMM_NULL)
                MPI_Comm_free(&small_comm);
            if (big_comm != MPI_COMM_NULL)
                MPI_Comm_free(&big_comm);
            free(Recv_set);
            free(set_small);
            free(set_large);
            return new_list_size;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: %s <inputfile> <outputfile> <pivot (1|2|3)>\n", argv[0]);
        return 1;
    }

    char *inputFile = argv[1];
    char *outputFile = argv[2];
    int pivotStrategy = atoi(argv[3]);
    if (pivotStrategy < 1 || pivotStrategy > 3)
    {
        fprintf(stderr, "Error: Pivot strategy must be 1, 2, or 3.\n");
        return 1;
    }
    int *elements = NULL;
    int num_elements = 0;
    int rank, num_proc;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        num_elements = read_input(inputFile, &elements);
        if (num_elements < 0)
        {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    if (num_proc == 1)
    {
        double start = MPI_Wtime();
        qsort(elements, num_elements, sizeof(int), compare);
        double execution_time = MPI_Wtime() - start;
        printf("Execution time: %f\n", execution_time);
        free(elements);
        MPI_Finalize();
        return 0;
    }

    // Broadcast the number of elements to all processes
    MPI_Bcast(&num_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    // Start timer
    double start = MPI_Wtime();
    int *my_elements = NULL;
    int my_count = distribute_from_root(elements, num_elements, &my_elements);

    qsort(my_elements, my_count, sizeof(int), compare);

    int new_count = global_sort(&my_elements, my_count, MPI_COMM_WORLD, pivotStrategy, 0);
    MPI_Barrier(MPI_COMM_WORLD);

    gather_on_root(elements, my_elements, new_count);

    MPI_Barrier(MPI_COMM_WORLD);
    double execution_time = MPI_Wtime() - start;

    if (rank == 0)
    {
        printf("Execution time: %f\n", execution_time);
        // check_and_print(elements, num_elements, outputFile);
    }

    free(my_elements);
    if (rank == 0)
    {
        free(elements);
    }
    MPI_Finalize();
    return 0;
}
