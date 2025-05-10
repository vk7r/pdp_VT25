#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "pivot.h"
#include "quicksort.h"

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
    FILE *file;
    if (NULL == (file = fopen(file_name, "w")))
    {
        perror("Couldn't open output file");
        return -2;
    }
    for (int i = 0; i < n; i++)
    {
        if (0 > fprintf(file, "%d ", elements[i]))
        {
            perror("Couldn't write to output file");
            return -2;
        }
    }
    if (0 > fprintf(file, "\n"))
    {
        perror("Couldn't write to output file");
        return -2;
    }
    if (0 != fclose(file))
    {
        perror("Warning: couldn't close output file");
    }
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
    *my_elements = malloc(my_count * sizeof(int));
    MPI_Scatterv(all_elements, send_counts, block_starts, MPI_INT,
                 *my_elements, my_count, MPI_INT,
                 0, MPI_COMM_WORLD);
    return my_count;
}

int comp(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int global_sort(int **elements, int n, MPI_Comm communicator, int pivot_strategy, int recursion_level)
{
    // printf("Rank %d: global_sort called with n = %d\n", rank, n);
    // int rank;
    // MPI_Comm_rank(communicator, &rank);
    // printf("Rank %d: global_sort called with n = %d\n", rank, n);

    // Select pivot and partition the elements
    // int pivot = select_pivot(pivot_strategy, *elements, n, communicator);
    // printf("Rank %d: Pivot = %d\n", rank, pivot);

    int larger_index = select_pivot(pivot_strategy, *elements, n, communicator);
    int size_set_small = larger_index;
    int size_set_large = n - larger_index;

    int set_small[size_set_small];
    int set_large[size_set_large];

    int size, rank;
    MPI_Group world_group;
    MPI_Comm_group(communicator, &world_group);
    MPI_Comm_size(communicator, &size);
    MPI_Comm_rank(communicator, &rank);
    MPI_Group small_group;
    MPI_Comm small_comm;
    MPI_Group big_group;
    MPI_Comm big_comm;

    // print all elements
    printf("Rank %d: start of function call: [", rank);
    for (int i = 0; i < n; i++)
    {
        printf("%d, ", (*elements)[i]);
    }
    printf("]\n\n");

    printf("Rank %d, RECURSIVE LVL %d\n", rank, recursion_level);

    int Recv_set_size = 0;
    int Recv_set[n];

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

    // printf("rank: %d, Set small: [", rank);
    // for (int i = 0; i < size_set_small; i++)
    // {
    //     printf("%d, ", set_small[i]);
    // }
    // printf("]\n");
    // printf("rank: %d, Set Large: [", rank);
    // for (int i = 0; i < size_set_large; i++)
    // {
    //     printf("%d, ", set_large[i]);
    // }
    // printf("]\n\n");
    // skapa nya comm grupper --> Small & Big  small_group = [ 0, 1, 2, 3 ] big_group = [ 4, 5, 6, 7 ],  small_group = [ 0, 1] big_group = [ 2, 3] small_group = [4,5 ] big_group = [ 6, 7 ]
    int middle = size / 2;
    // printf("Size: %d Rank %d: middle = %d\n", size, rank, middle);

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

    if (rank < middle) // HÄR
    {
        printf("Rank %d: I am in small comm, rec %d\n", rank, recursion_level);
        MPI_Comm_size(small_comm, &small_size);
        MPI_Comm_rank(small_comm, &small_rank);
        //////////////
        // Only send if there’s a corresponding partner in the big group
        if (small_rank < size - middle)
        {
            int partner = big_ranks[small_rank];
            // Send the size of the buffer to transfer // Receive the size of received buffer
            MPI_Sendrecv(&size_set_large, 1, MPI_INT, partner, 0,
                         &Recv_set_size, n, MPI_INT, partner, 0,
                         communicator, MPI_STATUS_IGNORE);

            // Print the local set
            printf("Rank %d, rec %d: Local set: [", rank, recursion_level);
            for (int i = 0; i < size_set_small; i++)
            {
                printf("%d, ", set_small[i]);
            }
            printf("]\n");

            // Swap set of numbers with "partner" process in other group.
            MPI_Sendrecv(set_large, size_set_large, MPI_INT, partner, 0,
                         Recv_set, Recv_set_size, MPI_INT, partner, 0,
                         communicator, MPI_STATUS_IGNORE);
            printf("Rank %d: Received from partner %d, rec %d\n", rank, partner, recursion_level);
            for (int i = 0; i < Recv_set_size; i++)
            {
                printf("Rank %d :Recv_set[%d] = %d\n", rank, i, Recv_set[i]);
            }
        }
        ///////////////////
        // Print the local set
        // printf("Rank %d: Local set: [", rank);
        // for (int i = 0; i < size_set_small; i++)
        // {
        //     printf("%d, ", set_small[i]);
        // }
        // printf("]\n");
        int new_list_size = size_set_small + Recv_set_size;
        // int new_local_list[new_list_size];
        int *new_local_list = malloc(new_list_size * sizeof(int));

        for (int i = 0; i < size_set_small; i++)
        {
            new_local_list[i] = set_small[i];
        }
        for (int i = 0; i < Recv_set_size; i++)
        {
            new_local_list[i + size_set_small] = Recv_set[i];
        }

        qsort(new_local_list, new_list_size, sizeof(int), comp);

        // printf("Rank %d: New local list: [", rank);
        // for (int i = 0; i < new_list_size; i++)
        // {
        //     printf("%d, ", new_local_list[i]);
        // }
        // printf("]\n");

        printf("Rank %d: big_size %d\n", rank, small_size);
        if (small_size != 1)
        {
            printf("Rank %d Entering recursive call...\n", rank);
            global_sort(&new_local_list, new_list_size, small_comm, pivot_strategy, recursion_level + 1);
        }
        else
        {
            printf("Rank %d: Finished with new local list: [", rank);
            for (int i = 0; i < new_list_size; i++)
            {
                printf("%d, ", new_local_list[i]);
            }
            printf("]\n");
        }
    }
    else
    {
        // printf("Rank %d: I am in big comm, rec %d\n", rank, recursion_level);
        MPI_Comm_size(big_comm, &big_size);
        MPI_Comm_rank(big_comm, &big_rank);
        //////////////////
        // Only send if there’s a corresponding partner in the small group
        if (big_rank < middle)
        {
            int partner = small_ranks[big_rank];
            MPI_Sendrecv(&size_set_small, 1, MPI_INT, partner, 0,
                         &Recv_set_size, n, MPI_INT, partner, 0,
                         communicator, MPI_STATUS_IGNORE);

            // Print the local set
            printf("Rank %d, rec %d: Local set: [", rank, recursion_level);
            for (int i = 0; i < size_set_large; i++)
            {
                printf("%d, ", set_large[i]);
            }
            printf("]\n");

            MPI_Sendrecv(set_small, size_set_small, MPI_INT, partner, 0,
                         Recv_set, Recv_set_size, MPI_INT, partner, 0,
                         communicator, MPI_STATUS_IGNORE);
            printf("Rank %d: Received from partner %d, rec: %d\n", rank, partner, recursion_level);
            for (int i = 0; i < Recv_set_size; i++)
            {
                printf("Recv_set[%d] = %d\n", i, Recv_set[i]);
            }
        }
        //////////////
        // Print the local set
        // printf("Rank %d: Local set: [", rank);
        // for (int i = 0; i < size_set_large; i++)
        // {
        //     printf("%d, ", set_large[i]);
        // }
        // printf("]\n");
        int new_list_size = size_set_large + Recv_set_size;
        // int new_local_list[new_list_size];
        int *new_local_list = malloc(new_list_size * sizeof(int));

        for (int i = 0; i < size_set_large; i++)
        {
            new_local_list[i] = set_large[i];
        }
        for (int i = 0; i < Recv_set_size; i++)
        {
            new_local_list[i + size_set_large] = Recv_set[i];
        }

        qsort(new_local_list, new_list_size, sizeof(int), comp);

        // printf("Rank %d: New local list: [", rank);
        // for (int i = 0; i < new_list_size; i++)
        // {
        //     printf("%d, ", new_local_list[i]);
        // }
        // printf("]\n");

        printf("\nBefore recursion: Rank %d: big_size %d\n", rank, big_size);
        if (big_size != 1)
        {
            printf("Rank %d Entering recursive call...\n", rank);
            global_sort(&new_local_list, new_list_size, big_comm, pivot_strategy, recursion_level + 1);
        }
        else
        {
            printf("Rank %d: Finished with new local list: [", rank);
            for (int i = 0; i < new_list_size; i++)
            {
                printf("%d, ", new_local_list[i]);
            }
            printf("]\n");
        }
        // free(new_local_list);
    }

    // Vilken grupp tillhör jag? 0 --> small_group

    // Print the received set
    // printf("Rank %d: Received set: [", rank);
    // for (int i = 0; i < Recv_set_size; i++)
    // {
    //     printf("%d, ", Recv_set[i]);
    // }
    // printf("]\n");

    // Create the new local list with the received elements

    // print the new local list

    // mitt_index = get_my_index();

    // Till vem ska jag skicka? grupp_min[mitt_index] == grupp_andra[mitt_index]

    // MPI_sendrecv(set_small, n, MPI_INT, set_large, n, MPI_INT, communicator);

    // int **new_local_list = sort shit idk broh

    // if(size != 1){global_sort(&new_local_list, n, my_new_group, pivot_strategy)}
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

    // Broadcast the number of elements to all processes
    MPI_Bcast(&num_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int *my_elements = NULL;
    int my_count = distribute_from_root(elements, num_elements, &my_elements);

    qsort(my_elements, my_count, sizeof(int), comp);

    int new_count = global_sort(&my_elements, my_count, MPI_COMM_WORLD, pivotStrategy, 0);
    // for (int i = 0; i < my_count; i++)
    // {
    //     printf("Rank %d: %d\n", rank, my_elements[i]);
    // }
    //  printf("Rank %d sorted %d elements\n", rank, my_count);
    free(my_elements);
    if (rank == 0)
    {
        free(elements);
    }
    MPI_Finalize();
    return 0;
}
