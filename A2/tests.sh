#!/bin/bash

echo "=== STRONG SCALING ==="
for P in 1 2 4 ; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./stencil input96.txt output96.txt 1 
done

echo "=== WEAK SCALING ==="
for P in 1 2 4 ; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./stencil input96.txt output96.txt $P 
done