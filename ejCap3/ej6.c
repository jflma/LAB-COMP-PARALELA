#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int n, p;
    double *A = NULL, *x = NULL, *y = NULL;
    double *local_A, *local_x, *local_y;
    int local_n;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    p = (int)sqrt(comm_sz);
    if (p * p != comm_sz) {
        if (my_rank == 0) {
            fprintf(stderr, "comm_sz debe ser un cuadrado perfecto\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (my_rank == 0) {
        printf("Ingrese orden de matriz n (debe ser divisible por %d): ", p);
        fflush(stdout);
        scanf("%d", &n);

        if (n % p != 0) {
            fprintf(stderr, "n debe ser divisible por sqrt(comm_sz)\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        A = malloc(n * n * sizeof(double));
        x = malloc(n * sizeof(double));
        y = calloc(n, sizeof(double));

        printf("Ingrese %d x %d matriz A:\n", n, n);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                scanf("%lf", &A[i*n + j]);

        printf("Ingrese vector x de longitud %d:\n", n);
        for (int i = 0; i < n; i++)
            scanf("%lf", &x[i]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    local_n = n / p;

    int dims[2] = {p, p};
    int periods[2] = {0, 0};
    MPI_Comm grid_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &grid_comm);

    int coords[2];
    MPI_Cart_coords(grid_comm, my_rank, 2, coords);
    int row = coords[0], col = coords[1];

    local_A = malloc(local_n * local_n * sizeof(double));
    local_x = malloc(local_n * sizeof(double));
    local_y = calloc(local_n, sizeof(double));

    if (my_rank == 0) {
        double *tmp = malloc(n*n * sizeof(double));
        for (int bi = 0; bi < p; bi++) {
            for (int bj = 0; bj < p; bj++) {
                for (int i = 0; i < local_n; i++) {
                    for (int j = 0; j < local_n; j++) {
                        int gi = bi*local_n + i;
                        int gj = bj*local_n + j;
                        tmp[(bi*p + bj)*local_n*local_n + i*local_n + j] = A[gi*n + gj];
                    }
                }
            }
        }
        MPI_Scatter(tmp, local_n*local_n, MPI_DOUBLE,
                    local_A, local_n*local_n, MPI_DOUBLE,
                    0, grid_comm);
        free(tmp);
    } else {
        MPI_Scatter(NULL, local_n*local_n, MPI_DOUBLE,
                    local_A, local_n*local_n, MPI_DOUBLE,
                    0, grid_comm);
    }

    if (row == col) {
        MPI_Scatter(x, local_n, MPI_DOUBLE,
                    local_x, local_n, MPI_DOUBLE,
                    0, grid_comm);
    }

    MPI_Bcast(local_x, local_n, MPI_DOUBLE, row, grid_comm);

    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_n; j++) {
            local_y[i] += local_A[i*local_n + j] * local_x[j];
        }
    }

    int diag_rank;
    MPI_Cart_rank(grid_comm, (int[]){row, row}, &diag_rank);
    MPI_Reduce(local_y, y + row*local_n, local_n, MPI_DOUBLE, MPI_SUM, diag_rank, grid_comm);

    if (my_rank == 0) {
        MPI_Gather(MPI_IN_PLACE, local_n, MPI_DOUBLE,
                   y, local_n, MPI_DOUBLE,
                   0, grid_comm);

        printf("Resultado y = A * x:\n");
        for (int i = 0; i < n; i++) {
            printf("%lf ", y[i]);
        }
        printf("\n");
        free(A); free(x); free(y);
    } else if (row == col) {
        MPI_Gather(y + row*local_n, local_n, MPI_DOUBLE,
                   NULL, local_n, MPI_DOUBLE,
                   0, grid_comm);
    }

    free(local_A);
    free(local_x);
    free(local_y);

    MPI_Finalize();
    return 0;
}
