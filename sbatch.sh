#!/bin/sh

##SBATCH --partition=ksu-chem-mri.q,ksu-gen-gpu.q

# Param 1 - ${1}: --bondEn: interaction energy
# Param 2 - ${2}: --iterations: total number of accepted moves
# Param 2 - ${3}: --split: frequency of saving files
# Param 2 - ${4}: --blocks: total blocks in the simulations block
# Param 2 - ${5}: --length: total units in the protein

# --runId: current duplicate job number

module load icc

${PROJECT_DIR}/src/program.o --bondEn ${1} --iterations ${2} --split ${3} --blocks ${4} --length ${5} --runId ${SLURM_ARRAY_TASK_ID}
