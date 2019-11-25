#!/bin/bash -l

#SBATCH --mail-type=ALL   # same as =BEGIN,FAIL,END
#SBATCH --mail-user=minhtien@ksu.edu
#SBATCH --mem-per-cpu=1G   # Memory per core, use --mem= for memory per node
#SBATCH --time=240:00:00   # Use the form DD-HH:MM:SS
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=nv1.1d1-E0.7L5
#SBATCH --export=ALL

##SBATCH --gres=gpu:1
##SBATCH --constraint=avx
#SBATCH --partition=ksu-chem-mri.q,ksu-gen-gpu.q
#SBATCH --array=1-10

#MIN=0.1
#INCREMENT=0.1
#CONCENTRATION=`echo "$MIN + $INCREMENT * ($SLURM_ARRAY_TASK_ID - 1)" | bc`

module load icc

./MC3D1 --bondEn 0.7 --iterations 10000000000 --split 2000000000 --blocks 70000 --length 5 --runId $SLURM_ARRAY_TASK_ID
