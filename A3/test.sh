#!/bin/bash
#SBATCH -A uppmax2025-2-247
#SBATCH -N 1
#SBATCH -n 8
#SBATCH -c 2
#SBATCH -t 00:00:30
#SBATCH --error=jobs/job.%j.err
#SBATCH --output=job.%j.out

echo "=== STRONG SCALING ==="
for P in  2 4 8 16; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./quicksort input.txt output.txt 2
done
