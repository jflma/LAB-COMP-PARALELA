#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int local_val, sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Each process starts with its rank as the value
    local_val = my_rank;
    sum = local_val;

    int steps = 0;
    while ((1 << steps) < comm_sz) steps++;

    for (int s = 0; s < steps; s++) {
        int partner = my_rank ^ (1 << s);
        if (partner < comm_sz) {
            int recv_val;
            MPI_Sendrecv(&sum, 1, MPI_INT, partner, 0,
                         &recv_val, 1, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += recv_val;
        }
    }

    printf("Process %d: Global sum = %d\n", my_rank, sum);

    MPI_Finalize();
    return 0;
}
