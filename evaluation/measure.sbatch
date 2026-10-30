#!/bin/bash
# SLURM job that runs evaluation/measure.sh alone on one avignon compute node.
# Submit it from the project folder; arguments are passed to measure.sh, e.g.
#   sbatch --nodelist=avignon1 evaluation/measure.sbatch soa objects
# Student jobs are limited to 10 minutes (QOS qosalumnos)
#SBATCH --job-name=grav-eval
#SBATCH --partition=avignon
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=00:10:00
#SBATCH --output=evaluation/measure-%j.log

bash evaluation/measure.sh "$@"
