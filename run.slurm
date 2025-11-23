#!/bin/bash
#SBATCH --job-name=pdp_matrix
#SBATCH --partition=draco
#SBATCH --nodes=4
#SBATCH --ntasks=64
#SBATCH --time=24:00:00
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

CSV_FILE=./experiments.csv
mapfile -t csv_lines < <(tail -n +2 "$CSV_FILE")
make
mkdir -p draco
cd draco

for line in "${csv_lines[@]}"; do
  [[ -z "$line" ]] && continue # skip empty

  IFS=',' read -r type size tasks replication <<<"$line"
  replication=${replication#.}

  echo "============================="
  echo "  Type: $type"
  echo "  Size: $size"
  echo "  Tasks: $tasks"
  echo "  Replication: $replication"
  nodes=$(($tasks / 4))
  machine_file="./nodes.slurm"
  srun -l hostname | sort -n | awk -v n="$nodes" '!seen[$2]++ {print $2 " slots=" n}' > $machine_file
  mpirun -np $tasks \
    -machinefile $machine_file \
    --mca btl ^openib \
    --mca btl_tcp_if_include eno2 \
    --bind-to none \
    ../$type $size $replication
done
