#!/bin/bash
#SBATCH -A uppmax2025-2-247
#SBATCH -N 1
#SBATCH -n 8
#SBATCH -c 2
#SBATCH -t 00:10:00
#SBATCH --error=jobs/job.%j.err
#SBATCH --output=job.%j.out


for strat in 1 2 3; do
    echo "____PRIVOT STRATEGY $strat ____"
    echo "=== STRONG SCALING ==="
    for P in 1 2 4 8 16; do
        echo "--- $P process(es) ---"
        mpirun --bind-to none -np $P ./quicksort /proj/uppmax2025-2-247/nobackup/A3/inputs/input125000000.txt output.txt $strat
    done

    echo "=== WEAK SCALING ==="
    input_files=(
        "/proj/uppmax2025-2-247/nobackup/A3/inputs/input125000000.txt"
        "/proj/uppmax2025-2-247/nobackup/A3/inputs/input250000000.txt"
        "/proj/uppmax2025-2-247/nobackup/A3/inputs/input500000000.txt"
        "/proj/uppmax2025-2-247/nobackup/A3/inputs/input1000000000.txt"
        "/proj/uppmax2025-2-247/nobackup/A3/inputs/input2000000000.txt"
    )

    i=0
    for P in 1 2 4 8 16; do
        echo "--- $P process(es) ---"
        input_file=${input_files[$i]}
        mpirun --bind-to none -np $P ./quicksort "$input_file" output.txt 2
        ((i++))
    done
done