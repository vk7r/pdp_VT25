#!/bin/bash
# The name of the account you are running in, mandatory.
#SBATCH -A uppmax2025-2-247
# and two tasks
#SBATCH -n 8
# and two cores per task
#SBATCH -c 2
# Request 5 minutes of runtime for the job
#SBATCH --time=00:00:16
# Set the names for the error and output files
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out

# mpicc -std=c99 -g -O2 -o treesum treeSum.c -lm

num_repeats=5

array_sizes=(100 1000 10000 100000 4194304)
num_processes=(1 2 4 8)

echo "Starting tests..."

for ((run=1; run<=num_repeats; run++)); do
    echo "======================="
    echo " FULL TEST RUN #$run "
    echo "======================="
    
    for size in "${array_sizes[@]}"; do
        echo "ARRAY SIZE: $size"
        for proc in "${num_processes[@]}"; do
            echo "Running with $proc processes..."
            mpirun -np $proc ./sum $size
        done
    done
done

echo "Testing complete"