#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int n;
    double *A = NULL, *x = NULL, *y = NULL;
    double *local_A, *local_x, *local_y;
    int local_cols;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (my_rank == 0) {
        printf("Ingrese orden de matriz n (debe ser divisible por comm_sz): ");
        fflush(stdout);
        scanf("%d", &n);

        A = malloc(n * n * sizeof(double));
        x = malloc(n * sizeof(double));
        y = malloc(n * sizeof(double));

        printf("Ingrese %d x %d matriz A:\n", n, n);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                scanf("%lf", &A[i*n + j]);

        printf("Ingrese vector x de longitud %d:\n", n);
        for (int i = 0; i < n; i++)
            scanf("%lf", &x[i]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_cols = n / comm_sz;
    local_A = malloc(n * local_cols * sizeof(double));
    local_x = malloc(local_cols * sizeof(double));
    local_y = calloc(n, sizeof(double));  

    if (my_rank == 0) {
        double *tmp = malloc(n * n * sizeof(double));
        for (int p = 0; p < comm_sz; p++) {
            for (int j = 0; j < local_cols; j++) {
                for (int i = 0; i < n; i++) {
                    tmp[p*n*local_cols + j*n + i] = A[i*n + p*local_cols + j];
                }
            }
        }
        MPI_Scatter(tmp, n*local_cols, MPI_DOUBLE,
                    local_A, n*local_cols, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
        free(tmp);
    } else {
        MPI_Scatter(NULL, n*local_cols, MPI_DOUBLE,
                    local_A, n*local_cols, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
    }

    MPI_Scatter(x, local_cols, MPI_DOUBLE,
                local_x, local_cols, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    for (int j = 0; j < local_cols; j++) {
        for (int i = 0; i < n; i++) {
            local_y[i] += local_A[j*n + i] * local_x[j];
        }
    }

    if (my_rank == 0) y = malloc(n * sizeof(double));
    MPI_Reduce(local_y, y, n, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("Resultado y = A * x:\n");
        for (int i = 0; i < n; i++) {
            printf("%lf ", y[i]);
        }
        printf("\n");
        free(A); free(x); free(y);
    }

    free(local_A);
    free(local_x);
    free(local_y);

    MPI_Finalize();
    return 0;
}
