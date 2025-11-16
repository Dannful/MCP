#!/bin/bash
#SBATCH --job-name=pdp_matrix
#SBATCH --partition=hype
#SBATCH --nodes=4
#SBATCH --ntasks=80
#SBATCH --time=24:00:00
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

CSV_FILE=./experiments.csv
mapfile -t csv_lines < <(tail -n +2 "$CSV_FILE")
make

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
  echo "hype1 slots=$nodes" >$machine_file
  {
    echo "hype2 slots=$nodes"
    echo "hype3 slots=$nodes"
    echo "hype5 slots=$nodes"
  } >>$machine_file
  mpirun -np $tasks \
    -machinefile $machine_file \
    --mca btl ^openib \
    --mca btl_tcp_if_include eno2 \
    --bind-to none \
    ./$type $size $replication
done
