#!/bin/bash

for strat in 1 2 3; do
    echo "____PRIVOT STRATEGY $strat ____"
    echo "=== STRONG SCALING ==="
    for P in 1 2 4; do
        echo "--- $P process(es) ---"
        mpirun --bind-to none -np $P ./quicksort /proj/uppmax2025-2-247/nobackup/A3/inputs/input125000000.txt output.txt $strat
    done

    echo "=== WEAK SCALING ==="
    input_files=(
        "input.txt"
        "input93.txt")

    i=0
    for P in 1 2 4; do
        echo "--- $P process(es) ---"
        input_file=${input_files[$i]}
        mpirun --bind-to none -np $P ./quicksort "$input_file" output.txt 2
        ((i++))
    done
done
