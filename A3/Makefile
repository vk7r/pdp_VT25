CC = mpicc
CFLAGS =  -O3 -g -Wall
BINS = quicksort
#-fsanitize=address -g

all: $(BINS)

quicksort: quicksort.h quicksort.c pivot.h pivot.c
	$(CC) $(CFLAGS) -fsanitize=address -g -o $@ quicksort.c pivot.c

clean:
	$(RM) $(BINS)

