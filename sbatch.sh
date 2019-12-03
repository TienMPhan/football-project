#!/bin/sh

#SBATCH --partition=ksu-chem-mri.q,ksu-gen-gpu.q
#SBATCH --array=1-10

# --bondEn: interaction energy
# --iterations: total number of accepted moves
# --split: frequency of saving files
# --blocks: total blocks in simulations block
# --length: total units in protein
# --runId: job number in array

${PROJECT_DIR}/program.o --bondEn ${BOND_ENERGY} --iterations 10000000000 --split 2000000000 --blocks 70000 --length 5 --runId $SLURM_ARRAY_TASK_ID
