###############################################################################
# Makefile for assignment 1, Parallel and Distributed Computing
###############################################################################

CC = mpicc
CFLAGS = -std=c99 -g -O2
LIBS = -lm

BIN = sum

all: $(BIN)

sum: scatterSum.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	$(RM) $(BIN)
