#!/bin/bash
#SBATCH -A uppmax2025-2-247
#SBATCH -N 1
#SBATCH -n 16
#SBATCH -c 1
#SBATCH -t 00:10:00
#SBATCH --error=jobs/job.%j.err
#SBATCH --output=job.%j.out

echo "=== STRONG SCALING ==="
for P in  2 4 8 16; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./quicksort /proj/uppmax2025-2-247/nobackup/A3/inputs/backwards/backwards_input250000000.txt output.txt 2
done
