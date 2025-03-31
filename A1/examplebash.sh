#!/bin/bash
# The name of the account you are running in, mandatory.
#SBATCH -A uppmax2025-2-247
# Asking for two nodes
#SBATCH -N 2
# and two tasks
#SBATCH -n 1
# and two cores per task
#SBATCH -c 2
# Request 5 minutes of runtime for the job
#SBATCH - -time=00:00:04
# Set the names for the error and output files
#SBATCH - -error=job.%J.err
#SBATCH - -output=job.%J.out

# Run with srun
mpirun ./summake 