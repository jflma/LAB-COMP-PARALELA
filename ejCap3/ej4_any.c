#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int local_val, sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    local_val = my_rank;  
    sum = local_val;

    int p = 1;
    while (p * 2 <= comm_sz) p *= 2;

    if (my_rank >= p) {
        MPI_Send(&sum, 1, MPI_INT, my_rank - p, 0, MPI_COMM_WORLD);
    } else {
        if (my_rank + p < comm_sz) {
            int extra_val;
            MPI_Recv(&extra_val, 1, MPI_INT, my_rank + p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += extra_val;
        }

        int steps = 0;
        while ((1 << steps) < p) steps++;

        for (int s = 0; s < steps; s++) {
            int partner = my_rank ^ (1 << s);
            int recv_val;
            MPI_Sendrecv(&sum, 1, MPI_INT, partner, 0,
                         &recv_val, 1, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += recv_val;
        }
    }


    if (my_rank < p) {
        for (int q = p; q < comm_sz; q++) {
            MPI_Send(&sum, 1, MPI_INT, q, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&sum, 1, MPI_INT, my_rank - p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    printf("Proceso %d: Global suma = %d\n", my_rank, sum);

    MPI_Finalize();
    return 0;
}
