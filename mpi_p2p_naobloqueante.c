// MPI Matrix Multiplication using Non-Blocking Communication
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void initialize_matrices(long n, double *A, double *B, double *C) {
  for (long i = 0; i < n * n; i++) {
    A[i] = i % 100;
    B[i] = (i % 100) + 1;
    C[i] = 0.0;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <size> <replication_index>\n", argv[0]);
    return 1;
  }
  int rank, size;
  long n = atol(argv[1]);
  char *replication_index = argv[2];
  char *type = argv[0];

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

  double t1_size = MPI_Wtime();
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  double t2_size = MPI_Wtime();

  char dir_path[256];
  sprintf(dir_path, "results/%s/%s/%d/%s", type, argv[1], size, replication_index);

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
  
  printf("type=%s, rank=%d, n=%ld, comm_size=%d, replication_index=%s, output_file=%s\n",
         type, rank, n, size, replication_index, filename);

  FILE *fp = fopen(filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file for rank %d at %s\n", rank, filename);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  fprintf(fp, "event,seconds\n");
  fprintf(fp, "MPI_Init,%lf\n", init_time);
  fprintf(fp, "MPI_Comm_rank,%lf\n", rank_time);
  fprintf(fp, "MPI_Comm_size,%lf\n", t2_size - t1_size);

  double *A, *B, *C;
  A = (double *)malloc(n * n * sizeof(double));
  B = (double *)malloc(n * n * sizeof(double));
  C = (double *)malloc(n * n * sizeof(double));

  if (rank == 0) {
    initialize_matrices(n, A, B, C);
  }

  double *local_A = (double *)malloc((n * n / size) * sizeof(double));
  double *local_C = (double *)malloc((n * n / size) * sizeof(double));

  MPI_Request request;

  double t1, t2;
  if (rank == 0)
    t1 = MPI_Wtime();

  if (rank == 0) {
    for (int i = 1; i < size; i++) {
      double t1 = MPI_Wtime();
      MPI_Isend(A + i * (n * n / size), n * n / size, MPI_DOUBLE, i, 0,
                MPI_COMM_WORLD, &request);
      double t2 = MPI_Wtime();
      fprintf(fp, "MPI_Isend,%lf\n", t2 - t1);
    }
    for (int i = 0; i < n * n / size; i++) {
      local_A[i] = A[i];
    }
  } else {
    double t1 = MPI_Wtime();
    MPI_Irecv(local_A, n * n / size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
              &request);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Irecv,%lf\n", t2 - t1);
    double t3 = MPI_Wtime();
    MPI_Wait(&request, MPI_STATUS_IGNORE);
    double t4 = MPI_Wtime();
    fprintf(fp, "MPI_Wait,%lf\n", t4 - t3);
  }

  {
    double t1 = MPI_Wtime();
    MPI_Ibcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD, &request);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Ibcast,%lf\n", t2 - t1);
  }
  {
    double t1 = MPI_Wtime();
    MPI_Wait(&request, MPI_STATUS_IGNORE);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Wait,%lf\n", t2 - t1);
  }

  for (long i = 0; i < n / size; i++) {
    for (long j = 0; j < n; j++) {
      local_C[i * n + j] = 0.0;
      for (long k = 0; k < n; k++) {
        local_C[i * n + j] += local_A[i * n + k] * B[k * n + j];
      }
    }
  }

  if (rank == 0) {
    for (int i = 0; i < n * n / size; i++) {
      C[i] = local_C[i];
    }
    for (int i = 1; i < size; i++) {
      double t1 = MPI_Wtime();
      MPI_Irecv(C + i * (n * n / size), n * n / size, MPI_DOUBLE, i, 1,
                MPI_COMM_WORLD, &request);
      double t2 = MPI_Wtime();
      fprintf(fp, "MPI_Irecv,%lf\n", t2 - t1);
      double t3 = MPI_Wtime();
      MPI_Wait(&request, MPI_STATUS_IGNORE);
      double t4 = MPI_Wtime();
      fprintf(fp, "MPI_Wait,%lf\n", t4 - t3);
    }
  } else {
    double t1 = MPI_Wtime();
    MPI_Isend(local_C, n * n / size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD,
              &request);
    double t2 = MPI_Wtime();
    fprintf(fp, "MPI_Isend,%lf\n", t2 - t1);
    double t3 = MPI_Wtime();
    MPI_Wait(&request, MPI_STATUS_IGNORE);
    double t4 = MPI_Wtime();
    fprintf(fp, "MPI_Wait,%lf\n", t4 - t3);
  }

  if (rank == 0) {
    t2 = MPI_Wtime();
    fprintf(fp, "ELAPSED_TIME,%lf\n", t2 - t1);
  }

  /*
      if (rank == 0) {
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