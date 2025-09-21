#!/bin/bash
#SBATCH --job-name=mtp
#SBATCH --partition=hype
#SBATCH --nodes=1
#SBATCH --ntasks=40
#SBATCH --time=24:00:00
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

CSV_FILE=./experiments.csv
mapfile -t csv_lines < <(tail -n +2 "$CSV_FILE")

source /home/intel/oneapi/vtune/2021.1.1/vtune-vars.sh

make collapse.mtp
make samples.mtp
make tasks.mtp

for line in "${csv_lines[@]}"; do
    [[ -z "$line" ]] && continue # skip empty

    IFS=',' read -r resolution otimization samples threads blocks <<< "$line"

    echo "============================="
    echo "  Resolution: $resolution"
    echo "  Otimization: $otimization"
    echo "  Samples: $samples"
    echo "  Threads: $threads"
    echo "  Blocks: $blocks"

    block=$(echo "$blocks" | sed 's/\.//')
    path="$(hostname)/$resolution/$otimization/$samples/$threads/$block"
    echo "Directory: $path"
    mkdir -p $path
    resolution=$(echo "$resolution" | sed 's/x/ /')
    export OMP_NUM_THREADS=$threads
    vtune -collect hpc-performance \
          -result-dir "$path" \
          ./$otimization.mtp $resolution $samples "$path/output.png" 2>&1 > "$path/logs.out"
    vtune -report summary \
          -result-dir "$path" \
          -format csv \
          -report-output "$path/vtune_reports.csv"
done
