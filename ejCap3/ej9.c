#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int n, local_n;
    int *block_vec, *cyclic_vec;
    double start, end, block_to_cyclic_time, cyclic_to_block_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (my_rank == 0) {
        if (argc != 2) {
            fprintf(stderr, "Usa: mpiexec -n <p> %s <n>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        n = atoi(argv[1]);
        if (n % comm_sz != 0) {
            fprintf(stderr, "n debe ser divisible por comm_sz\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    local_n = n / comm_sz;

    block_vec = malloc(local_n * sizeof(int));
    cyclic_vec = malloc(local_n * sizeof(int));

    for (int i = 0; i < local_n; i++) {
        block_vec[i] = my_rank * local_n + i;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    int *temp_recv = malloc(local_n * sizeof(int));
    for (int i = 0; i < local_n; i++) {
        int global_index = my_rank * local_n + i;
        int dest = global_index % comm_sz;
        int local_index = global_index / comm_sz;
        MPI_Send(&block_vec[i], 1, MPI_INT, dest, local_index, MPI_COMM_WORLD);
    }

    for (int i = 0; i < local_n; i++) {
        MPI_Status status;
        MPI_Recv(&cyclic_vec[i], 1, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    block_to_cyclic_time = end - start;

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    for (int i = 0; i < local_n; i++) {
        int global_index = my_rank + i * comm_sz;
        int dest = global_index / local_n;  
        int local_index = global_index % local_n;
        MPI_Send(&cyclic_vec[i], 1, MPI_INT, dest, local_index, MPI_COMM_WORLD);
    }

    for (int i = 0; i < local_n; i++) {
        MPI_Status status;
        MPI_Recv(&block_vec[i], 1, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    cyclic_to_block_time = end - start;

    double max_block_to_cyclic, max_cyclic_to_block;
    MPI_Reduce(&block_to_cyclic_time, &max_block_to_cyclic, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&cyclic_to_block_time, &max_cyclic_to_block, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("Costo de Redistribucion:\n");
        printf("Block → Cyclic: %f segundos\n", max_block_to_cyclic);
        printf("Cyclic → Block: %f segundos\n", max_cyclic_to_block);
    }

    free(block_vec);
    free(cyclic_vec);
    free(temp_recv);

    MPI_Finalize();
    return 0;
}
