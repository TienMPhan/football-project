#!/bin/bash -l

#SBATCH --mail-type=ALL   # same as =BEGIN,FAIL,END
#SBATCH --mail-user=minhtien@ksu.edu
#SBATCH --mem-per-cpu=1G   # Memory per core, use --mem= for memory per node
#SBATCH --time=128:00:00   # Use the form DD-HH:MM:SS
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=fb-08
#SBATCH --export=ALL

##SBATCH --gres=gpu:1
##SBATCH --constraint=avx
#SBATCH --partition=ksu-chem-mri.q,ksu-gen-gpu.q
#SBATCH --array=1-10

#MIN=0.1
#INCREMENT=0.1
#CONCENTRATION=`echo "$MIN + $INCREMENT * ($SLURM_ARRAY_TASK_ID - 1)" | bc`

./MC3D 0.8 3000000000 12000 5 $SLURM_ARRAY_TASK_ID
