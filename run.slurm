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

cd mcp
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

    path=$(hostname)/$resolution/$otimization/$samples/$threads
    mkdir -p $path
    OMP_NUM_THREADS=$threads $otimization.mtp $resolution $samples 2>&1 > $path/$blocks.out
done
