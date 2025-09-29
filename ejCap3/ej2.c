#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    long long int total_tosses, local_tosses;
    long long int number_in_circle = 0;
    long long int local_number_in_circle = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (my_rank == 0) {
        printf("Introduzca el numero total de tiradas: ");
        fflush(stdout);
        scanf("%lld", &total_tosses);
    }

    MPI_Bcast(&total_tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    local_tosses = total_tosses / comm_sz;
    if (my_rank == comm_sz - 1) {
        local_tosses += total_tosses % comm_sz;  
    }

    unsigned int seed = (unsigned int)(time(NULL) + my_rank * 1000);

    for (long long int toss = 0; toss < local_tosses; toss++) {
        double x = (rand_r(&seed) / (double)RAND_MAX) * 2.0 - 1.0;
        double y = (rand_r(&seed) / (double)RAND_MAX) * 2.0 - 1.0;
        double distance_squared = x * x + y * y;
        if (distance_squared <= 1.0) {
            local_number_in_circle++;
        }
    }

    MPI_Reduce(&local_number_in_circle, &number_in_circle, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        double pi_estimate = 4.0 * (double)number_in_circle / ((double) total_tosses);
        printf("Estimado pi = %lf\n", pi_estimate);
    }

    MPI_Finalize();
    return 0;
}
