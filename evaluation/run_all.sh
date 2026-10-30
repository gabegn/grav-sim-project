#!/usr/bin/env bash
# Submits every measurement of evaluation/measure.sh as four SLURM jobs on one avignon node.
# Student jobs are limited to 10 minutes, so the work is split by version and experiment.
# Expected durations, from times measured on avignon1 (Intel i5-4460):
#   aos official iterations ~3 min, aos objects ~3 min,
#   soa official iterations ~4 min, soa objects ~4.5 min
# The jobs ask for the whole node (--exclusive), so they run one after the other and never
# disturb each other's measurements. All of them append to evaluation/results.csv; an
# existing results.csv is kept as a backup instead of being overwritten.
# A student can have at most 3 jobs submitted at a time, so the script waits for a free
# slot before submitting the next job (keep the terminal open until it finishes).
#
# Usage (from anywhere): evaluation/run_all.sh [node]   (default node: avignon1)
set -euo pipefail

cd "$(dirname "$0")/.."
node="${1:-avignon1}"
results="evaluation/results.csv"

if [[ -f "${results}" ]]; then
  backup="evaluation/results-$(date +%Y%m%d-%H%M%S).csv.bak"
  mv "${results}" "${backup}"
  echo "Previous results kept in ${backup}"
fi

max_jobs=3
for job in "aos official iterations" "aos objects" "soa official iterations" "soa objects"; do
  while [[ $(squeue -h -u "${USER}" | wc -l) -ge ${max_jobs} ]]; do
    echo "Waiting for a free job slot (at most ${max_jobs} jobs at a time)..."
    sleep 30
  done
  echo "Submitting: ${job}"
  # The job description is split on purpose: its words are the arguments of measure.sh
  # shellcheck disable=SC2086
  sbatch --nodelist="${node}" evaluation/measure.sbatch ${job}
done

echo "Follow the jobs with: squeue -u \$USER (about 15 minutes in total)"
