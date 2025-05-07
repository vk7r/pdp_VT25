#!/bin/bash
#SBATCH -A uppmax2025-2-247
#SBATCH -N 1
#SBATCH -n 8
#SBATCH -c 2
#SBATCH -t 00:00:30
#SBATCH --error=jobs/job.%j.err
#SBATCH --output=job.%j.out

echo "=== STRONG SCALING ==="
for P in 1 2 4 8 16; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./stencil /proj/uppmax2025-2-247/A2/input8000000.txt output_scaling.txt 10
done

echo "=== WEAK SCALING ==="
# Input files for increasing problem sizes
input_files=("/proj/uppmax2025-2-247/A2/input1000000.txt" "/proj/uppmax2025-2-247/A2/input2000000.txt" "/proj/uppmax2025-2-247/A2/input4000000.txt" "/proj/uppmax2025-2-247/A2/input8000000.txt")

i=0
for P in 1 2 4 8; do
    echo "--- $P process(es) ---"
    mpirun --bind-to none -np $P ./stencil "${input_files[$i]}" output_scaling.txt 10
    ((i++))
done

