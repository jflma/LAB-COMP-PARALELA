#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define TAG 0
#define NTRIALS 100000

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int msg = 1;
    MPI_Status status;
    clock_t start_clock, end_clock;
    double start_wtime, end_wtime;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (comm_sz != 2) {
        if (my_rank == 0)
            fprintf(stderr, "Corre con 2 procesos\n");
        MPI_Finalize();
        return 1;
    }

    // ---------- C clock ----------
    if (my_rank == 0) {
        start_clock = clock();
        for (int i = 0; i < NTRIALS; i++) {
            MPI_Send(&msg, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
            MPI_Recv(&msg, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD, &status);
        }
        end_clock = clock();

        double elapsed = ((double)(end_clock - start_clock)) / CLOCKS_PER_SEC;
        printf("clock() = %f segundos\n", elapsed);
    } else {
        for (int i = 0; i < NTRIALS; i++) {
            MPI_Recv(&msg, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&msg, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // ---------- MPI_Wtime ----------
    if (my_rank == 0) {
        start_wtime = MPI_Wtime();
        for (int i = 0; i < NTRIALS; i++) {
            MPI_Send(&msg, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
            MPI_Recv(&msg, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD, &status);
        }
        end_wtime = MPI_Wtime();

        double elapsed = end_wtime - start_wtime;
        printf("MPI_Wtime = %f segundos\n", elapsed);
    } else {
        for (int i = 0; i < NTRIALS; i++) {
            MPI_Recv(&msg, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&msg, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
