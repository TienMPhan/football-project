#!/bin/bash

ENERGY=(0.5 0.55)

for E in ${ENERGY[*]}
do
    sed "s/REPLACE/$E/" submit_energy.sh > submit_E${E}.sh
    sbatch < submit_E${E}.sh
done
