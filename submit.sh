#!/bin/sh
export PROTEIN_PROJECT_DIR=$(dirname $(realpath $0)) # directory where this script (submit.sh) is located
export PROTEIN_OUTPUT_DIR=${PROTEIN_PROJECT_DIR}/out # directory for program output (files, stdout, etc)

# default variable values
default_job_mailing_email=$(whoami)@ksu.edu
default_job_mailing_type=ALL
default_node_count=1
default_core_count_per_node=1
default_max_job_time=00-12:00:00
default_submit_single_job=false
default_duplicate_jobs_count=10

default_arg_bond_energy=0.5
default_arg_iterations=10000000000
default_arg_splits=2000000000
default_arg_blocks=70000
default_arg_length=5
default_job_write_id=1
default_dimensions=4
default_xm=200
default_ym=500
default_zm=200

# variable declarations
job_mailing_email=$default_job_mailing_email
job_mailing_type=$default_job_mailing_type
export PROTEIN_NODE_COUNT=$default_node_count
export PROTEIN_CORE_COUNT_PER_NODE=$default_core_count_per_node
max_job_time=$default_max_job_time
submit_single_job=$default_submit_single_job
duplicate_jobs_count=$default_duplicate_jobs_count

arg_bond_energy=$default_arg_bond_energy
arg_iterations=$default_arg_iterations
arg_splits=$default_arg_splits
arg_blocks=$default_arg_blocks
arg_length=$default_arg_length
arg_write_id=$default_job_write_id
arg_dimensions=$default_dimensions
arg_Xm=$default_xm
arg_Ym=$default_ym
arg_Zm=$default_zm

show_script_help() {
    echo "
    --help                      show script information
	--email						set the email for job status to be sent to (default: ${default_job_mailing_email})
	--email-type				set the types of job status that are sent to the given email (default: $default_job_mailing_type)
	--nodes | -n				set the number of nodes (default: ${default_node_count})
	--cores-per-node | -c		set the number of cores	per node (default: ${default_core_count_per_node})
    --max-job-time              set the maximum time a job will run before being automatically cancelled (format: DD-HH:MM:SS) (default: $default_max_job_time)
	--single | -s				flag to only submit one job with the given node count and core count per node (default: ${default_submit_single_job})
    --duplicate-jobs            set the number of times each job will run (default: ${default_duplicate_jobs_count})

    --bond-energy               set the bond energy for one job submission (default: $default_arg_bond_energy)
    --iterations                set the total number of accepted moves (default: $default_arg_iterations)
    --splits                    set the frequency of saving files (default: $default_arg_splits)
    --blocks                    set the total blocks in the simulations block (default: $default_arg_blocks)
    --length                    set the total units in the protein (default: $default_arg_length)
"
}

submit_slurm_job() {
    export PROTEIN_JOB_OUTPUT_DIR=${PROTEIN_OUTPUT_DIR}/En-${arg_bond_energy}
    mkdir -p $PROTEIN_JOB_OUTPUT_DIR
    slurm_output_file=${PROTEIN_JOB_OUTPUT_DIR}/slurm-%j_${arg_bond_energy}L${arg_length}-run${arg_write_id}.out
    if [ ! -z "$job_mailing_email" ] && [ ! -z $job_mailing_type ]; then
        slurm_cmd_mailing_flags="--mail-user=${job_mailing_email} --mail-type=${job_mailing_type}"
    fi

    overall_core_count=$(($PROTEIN_NODE_COUNT * $PROTEIN_CORE_COUNT_PER_NODE))
    Xm_chunk_size=$((($arg_Xm / $PROTEIN_CORE_COUNT_PER_NODE) + ($arg_Xm % $PROTEIN_CORE_COUNT_PER_NODE)))
    blocks_chunk_size=$((($arg_blocks / $PROTEIN_CORE_COUNT_PER_NODE) + ($arg_blocks % $PROTEIN_CORE_COUNT_PER_NODE)))

    mem_root_lattice_buffer=$((($arg_Xm * $arg_Ym * $arg_Zm * 14) + (12 + ($arg_Xm * $arg_Ym * $arg_Zm * 35))))
    mem_root_coordinates_buffer=$(($arg_blocks * 83))
    mem_per_cpu_lattice_buffer=$((($Xm_chunk_size * $arg_Ym * $arg_Zm * 14) + ($Xm_chunk_size * $arg_Ym * $arg_Zm * 35)))
    mem_per_cpu_coordinates_buffer=$(($blocks_chunk_size * 83))

    mem_root=$((($overall_core_count * 16) + ($arg_blocks * $arg_dimensions * 4) + ($arg_Xm * $arg_Ym * $arg_Zm * 4)))
    if test $mem_root_lattice_buffer -gt $mem_root_coordinates_buffer; then
        mem_root=$(($mem_root + $mem_root_lattice_buffer))
        mem_per_cpu=$mem_per_cpu_lattice_buffer
    else
        mem_root=$(($mem_root + $mem_root_coordinates_buffer))
        mem_per_cpu=$mem_per_cpu_coordinates_buffer
    fi
    memory=$(((($mem_root + ($overall_core_count * $mem_per_cpu)) / 1000000) + 512)) # memory per core in megabytes (M or MB)

    sbatch --constraint=elves --output=${slurm_output_file} --error=${slurm_output_file} --job-name=nv1.1d1-E${arg_bond_energy}L5 \
        --mem=${memory}M --time=${max_job_time} --nodes=${PROTEIN_NODE_COUNT} --ntasks-per-node=${PROTEIN_CORE_COUNT_PER_NODE} --array=1-${duplicate_jobs_count} \
        ${slurm_cmd_mailing_flags} ${PROTEIN_PROJECT_DIR}/sbatch.sh $arg_bond_energy $arg_iterations $arg_splits $arg_blocks $arg_length $arg_Xm $arg_Ym $arg_Zm \
        $arg_dimensions $arg_write_id
}

mass_submit_slurm_job() {
    energy_range=(0.5 0.55)
    for e in ${energy_range[*]}; do
        arg_bond_energy=$e
        submit_slurm_job
    done
}

# parse script parameters
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
        job_mailing_type=$2
        shift
        ;;
    --nodes | -n)
        export PROTEIN_NODE_COUNT=$2
        shift
        ;;
    --cores-per-node | -c)
        export PROTEIN_CORE_COUNT_PER_NODE=$2
        shift
        ;;
    --max-job-time)
        max_job_time=$2
        shift
        ;;
    --single | -s)
        submit_single_job=true
        ;;
    --duplicate-jobs)
        duplicate_jobs_count=$2
        shift
        ;;
    --bond-energy)
        arg_bond_energy=$2
        shift
        ;;
    --iterations)
        arg_iterations=$2
        shift
        ;;
    --splits)
        arg_splits=$2
        shift
        ;;
    --blocks)
        arg_blocks=$2
        shift
        ;;
    --length)
        arg_length=$2
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
    submit_slurm_job
else
    mass_submit_slurm_job
fi
