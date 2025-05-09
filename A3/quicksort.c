#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

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


// Returns its own new comm group (?)
int create_comm_groups(MPI_Comm communicator)
{
    size,rank; 
    MPI_Comm_size(communicator, &size);
    MPI_Comm_rank(communicator, &rank);

    middle = rank / 2; // 8 / 2 = 4
    buddy = rank % middle;

    MPI_Group new_group;
    MPI_Comm new_comm;

    int *small_group[];
    int *big_group[];

    if(rank == 0){
        // SKAPA GRUPPER
        for (int i = 0; i < size; i++)
        {
            if (i < middle)
            {
                small_group[i] = i;
            }
            else
            {
                big_group[i] = i;
            }
        }
    }    

    //broadcast small_group
    //broadcast big_group

    
    int ranks[2] = {0, 1}; // Only want ranks 0 and 1 in new group

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);          // Get group from default communicator
    MPI_Group_incl(world_group, 2, ranks, &new_group);     // Create a new group with selected ranks
    MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm); // Create a communicator from the group

    small = [ 0, 1, 2, 3 ] big = [ 4, 5, 6, 7 ]

                                 small[i]-- > big[i] i++;
    // på small
    small = [ 0, 1 ] big = [ 2, 3 ]

                           small[i]-- > big[i] i++;
}

int global_sort(int **elements, int n, MPI_Comm communicator, int pivot_strategy)
{
    int larger_index = select_pivot(pivot_strategy, *elements, n, communicator);

    int set_small[n];
    int set_large[n];

    for (int i = 0; i < n; i++)
    {
        if ((*elements)[i] < (*elements)[larger_index])
        {
            set_small[i] = (*elements)[i];
        }
        else
        {
            set_large[i] = (*elements)[i];
        }
    }

    // skapa nya comm grupper --> Small & Big  small_group = [ 0, 1, 2, 3 ] big_group = [ 4, 5, 6, 7 ],  small_group = [ 0, 1] big_group = [ 2, 3] small_group = [4,5 ] big_group = [ 6, 7 ]

    // Vilken grupp tillhör jag? 0 --> small_group 

    // mitt_index = get_my_index();

    // Till vem ska jag skicka? grupp_min[mitt_index] == grupp_andra[mitt_index]

    // int **new_local_list = sort shit idk broh

    //if(size != 1){global_sort(&new_local_list, n, my_new_group, pivot_strategy)}
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
    int new_count = global_sort(&my_elements, my_count, MPI_COMM_WORLD, pivotStrategy);
    for (int i = 0; i < my_count; i++)
    {
        printf("Rank %d: %d\n", rank, my_elements[i]);
    }
    printf("Rank %d sorted %d elements\n", rank, my_count);
    free(my_elements);
    if (rank == 0)
    {
        free(elements);
    }
    MPI_Finalize();
    return 0;
}
