// MPI Matrix Multiplication using Point-to-Point Communication
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void initialize_matrices(int n, double *A, double *B, double *C) {
  for (int i = 0; i < n * n; i++) {
    A[i] = i % 100;
    B[i] = (i % 100) + 1;
    C[i] = 0.0;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <size> <num_tasks> <replication_index>\n",
            argv[0]);
    return 1;
  }
  int rank, size, n = atoi(argv[1]);
  char *num_tasks = argv[2];
  char *replication_index = argv[3];
  char *type = "mpi_coletiva";

  double init_time, rank_time;
  double temp_t1, temp_t2;

  temp_t1 = MPI_Wtime();
  MPI_Init(&argc, &argv);
  temp_t2 = MPI_Wtime();
  init_time = temp_t2 - temp_t1;

  temp_t1 = MPI_Wtime();
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  temp_t2 = MPI_Wtime();
  rank_time = temp_t2 - temp_t1;

  char dir_path[256];
  sprintf(dir_path, "results/%s/%s/%s/%s", type, argv[1], num_tasks,
          replication_index);

  if (rank == 0) {
    char command[300];
    sprintf(command, "mkdir -p %s", dir_path);
    if (system(command) != 0) {
      fprintf(stderr, "Error creating directory %s\n", dir_path);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  char filename[300];
  sprintf(filename, "%s/%d.out", dir_path, rank);

  printf("type=%s, rank=%d, size=%s, num_tasks=%s, replication_index=%s, "
         "output_file=%s\n",
         type, rank, argv[1], num_tasks, replication_index, filename);

  FILE *fp = fopen(filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file for rank %d at %s\n", rank, filename);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  fprintf(fp, "event,seconds\n");
  fprintf(fp, "MPI_Init,%lf\n", init_time);
  fprintf(fp, "MPI_Comm_rank,%lf\n", rank_time);

  {
    double t1 = MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Comm_size,%lf\n", t2 - t1);
  }

  double *A, *B, *C;
  A = (double *)malloc(n * n * sizeof(double));
  B = (double *)malloc(n * n * sizeof(double));
  C = (double *)malloc(n * n * sizeof(double));

  if (rank == 0) {
    initialize_matrices(n, A, B, C);
  }

  double *local_A = (double *)malloc((n * n / size) * sizeof(double));
  double *local_C = (double *)malloc((n * n / size) * sizeof(double));

  double t1, t2;
  if (rank == 0)
    t1 = MPI_Wtime();

  {
    double t1 = MPI_Wtime();
    MPI_Scatter(A, n * n / size, MPI_DOUBLE, local_A, n * n / size, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Scatter,%lf\n", t2 - t1);
  }
  {
    double t1 = MPI_Wtime();
    MPI_Bcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Bcast,%lf\n", t2 - t1);
  }

  for (int i = 0; i < n / size; i++) {
    for (int j = 0; j < n; j++) {
      local_C[i * n + j] = 0.0;
      for (int k = 0; k < n; k++) {
        local_C[i * n + j] += local_A[i * n + k] * B[k * n + j];
      }
    }
  }

  {
    double t1 = MPI_Wtime();
    MPI_Gather(local_C, n * n / size, MPI_DOUBLE, C, n * n / size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Gather,%lf\n", t2 - t1);
  }

  if (rank == 0) {
    t2 = MPI_Wtime();
    fprintf(fp, "ELAPSED_TIME,%lf\n", t2 - t1);
  }

  /*    if (rank == 0) {
          fprintf(fp, "Result Matrix:\n");
          for (int i = 0; i < n; i++) {
              for (int j = 0; j < n; j++) {
                  fprintf(fp, "%f ", C[i * n + j]);
              }
              fprintf(fp, "\n");
          }
      }
  */
  free(A);
  free(B);
  free(C);
  free(local_A);
  free(local_C);

  {
    double t1 = MPI_Wtime();
    MPI_Finalize();
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Finalize,%lf\n", t2 - t1);
  }
  fclose(fp);
  return 0;
}
