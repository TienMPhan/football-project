#!/bin/sh
export PROJECT_DIR=$(dirname $(realpath $0))
export OUTPUT_DIR=${PROJECT_DIR}/out
slurmOutputFile=${OUTPUT_DIR}/slurm-%j.out

default_job_mailing_email=$(whoami)@ksu.edu
default_job_mailing_type=ALL
default_node_count=1
default_core_count_per_node=1
default_submit_single_job=false
default_bond_energy=0.5

job_mailing_email=$default_job_mailing_email
job_mailing_type=$default_job_mailing_type
export NODE_COUNT=$default_node_count
export CORE_COUNT_PER_NODE=$default_core_count_per_node
submit_single_job=$default_submit_single_job
export BOND_ENERGY=$default_bond_energy

show_script_help() {
    echo "
flags (optional):
	--email						set the email for job status to be sent to (default: ${default_job_mailing_email})
	--email-type				set the types of job status that are sent to the given email (default: $default_job_mailing_type)
	--nodes | -n				set the number of nodes (default: ${default_node_count})
	--cores-per-node | -c		set the number of cores	per node (default: ${default_core_count_per_node})
	--single | -s				flag to only submit one job with the given node count and core count per node (default: ${default_submit_single_job})
    --bond-energy               set the bond energy for one job submission (default: $default_bond_energy)
"
}

submit_job() {
    # Param #1 - ${1} - # of nodes
    # Param #2 - ${2} - # of tasks/cores per node
    # Param #3 - ${3} - Bond energy
    Xm=200
    Ym=500
    Zm=200
    dimensions=4
    memory=$(((($BLOCKS * $dimensions * 256) + ($Xm * $Ym * $Zm * 32) / 1000000) + 512))
    if [ ! -z "$job_mailing_email" ] && [ ! -z $user_email_type ]; then
        sbatch --constraint=elves --mem-per-cpu=${memory}M --time=0-12:00:00 --nodes=${1} --ntasks-per-node=${2} \
            --switches=1 --output=${slurmOutputFile} --error=${slurmOutputFile} --mail-user=${job_mailing_email} \
            --mail-type=${user_email_type} ${PROJECT_DIR}/sbatch.sh
    else
        sbatch --constraint=elves --mem-per-cpu=${memory}M --time=0-12:00:00 --nodes=${1} --ntasks-per-node=${2} \
            --switches=1 --output=${slurmOutputFile} --error=${slurmOutputFile} --job-name=nv1.1d1-E${3}L5 ${PROJECT_DIR}/sbatch.sh
    fi
}

mass_submit_job() {
    ENERGY=(0.5 0.55)
    for E in ${ENERGY[*]}; do
        export BOND_ENERGY=$E
        submit_job $NODE_COUNT $CORE_COUNT_PER_NODE $BOND_ENERGY
    done
}

while [ ! $# -eq 0 ]; do
    case "$1" in
    --help)
        show_script_help
        exit 0
        ;;
    --email)
        job_mailing_email=$2
        shift
        ;;
    --email-type)
        user_email_type=$2
        shift
        ;;
    --cores-per-node | -c)
        export CORE_COUNT_PER_NODE=$2
        shift
        ;;
    --nodes | -n)
        export NODE_COUNT=$2
        shift
        ;;
    --single | -s)
        submit_single_job=true
        ;;
    --bond-energy)
        export BOND_ENERGY=$2
        shift
        ;;
    *)
        show_script_help
        exit 1
        ;;
    esac
    shift
done

if [ $submit_single_job = true ]; then
    submit_job $NODE_COUNT $CORE_COUNT_PER_NODE $BOND_ENERGY
else
    mass_submit_job
fi
