CC = mpicc
CFLAGS =  -O3 -g #-Wall
BINS = quicksort

all: $(BINS)

quicksort: quicksort.h quicksort.c pivot.h pivot.c
	$(CC) $(CFLAGS) -o $@ quicksort.c pivot.c

clean:
	$(RM) $(BINS)

