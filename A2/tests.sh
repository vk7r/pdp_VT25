#!/bin/bash

echo "=== STRONG SCALING ==="
for P in 1 2 4 8 16; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./stencil input96.txt output_scaling.txt 4
done

echo "=== WEAK SCALING ==="
# Input files for increasing problem sizes
input_files=("input96.txt" "input1000000.txt" "input2000000.txt" "input4000000.txt" "input8000000.txt")

i=0
for P in 1 2 4 8 16; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./stencil "${input_files[$i]}" output_scaling.txt 4
    ((i++))
done
