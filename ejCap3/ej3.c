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

    int partner, step = 1;
    while (step < comm_sz) {
        if (my_rank % (2 * step) == 0) {
            partner = my_rank + step;
            int recv_val;
            MPI_Recv(&recv_val, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += recv_val;
        } else {
            partner = my_rank - step;
            MPI_Send(&sum, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            break; 
        }
        step *= 2;
    }

    if (my_rank == 0) {
        printf("Global suma = %d\n", sum);
    }

    MPI_Finalize();
    return 0;
}
