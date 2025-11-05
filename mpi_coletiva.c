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
  int rank, size, n = atoi(argv[1]);
  printf("event,seconds\n");
  {
    double t1 = MPI_Wtime();
    MPI_Init(&argc, &argv);
    double t2 = MPI_Wtime();
    printf("MPI_Init,%lf\n", t2 - t1);
  }
  {
    double t1 = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double t2 = MPI_Wtime();
    printf("MPI_Comm_rank,%lf\n", t2 - t1);
  }
  {
    double t1 = MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double t2 = MPI_Wtime();
    printf("MPI_Comm_size,%lf\n", t2 - t1);
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
    printf("MPI_Scatter,%lf\n", t2 - t1);
  }
  {
    double t1 = MPI_Wtime();
    MPI_Bcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    double t2 = MPI_Wtime();
    printf("MPI_Bcast,%lf\n", t2 - t1);
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
    printf("MPI_Gather,%lf\n", t2 - t1);
  }

  if (rank == 0) {
    t2 = MPI_Wtime();
    printf("ELAPSED_TIME,%lff\n", t2 - t1);
  }

  /*    if (rank == 0) {
          printf("Result Matrix:\n");
          for (int i = 0; i < n; i++) {
              for (int j = 0; j < n; j++) {
                  printf("%f ", C[i * n + j]);
              }
              printf("\n");
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
    printf("MPI_Finalize,%lf\n", t2 - t1);
  }
  return 0;
}
