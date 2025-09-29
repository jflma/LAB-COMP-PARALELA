#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int data_count = 20;
    float data[] = {1.3,2.9,0.4,0.3,1.3,4.4,1.7,0.4,3.2,0.3,
                    4.9,2.4,3.1,4.4,3.9,0.4,4.2,4.5,4.9,0.9};
    float min_meas = 0.0, max_meas = 5.0;
    int bin_count = 5;
    int *bin_counts = NULL;
    float bin_width;

    int local_n;
    float *local_data;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    bin_width = (max_meas - min_meas) / bin_count;

    if (my_rank == 0) {
        bin_counts = (int*)calloc(bin_count, sizeof(int));
    }

    local_n = data_count / comm_sz;
    if (my_rank == comm_sz - 1) {
        local_n += data_count % comm_sz; 
    }

    local_data = (float*)malloc(local_n * sizeof(float));

    int *sendcounts = NULL;
    int *displs = NULL;
    if (my_rank == 0) {
        sendcounts = (int*)malloc(comm_sz * sizeof(int));
        displs = (int*)malloc(comm_sz * sizeof(int));
        int base = data_count / comm_sz;
        int rem = data_count % comm_sz;
        int offset = 0;
        for (int i = 0; i < comm_sz; i++) {
            sendcounts[i] = base + (i == comm_sz - 1 ? rem : 0);
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }

    MPI_Scatterv(data, sendcounts, displs, MPI_FLOAT,
                 local_data, local_n, MPI_FLOAT,
                 0, MPI_COMM_WORLD);

    int *local_bins = (int*)calloc(bin_count, sizeof(int));
    for (int i = 0; i < local_n; i++) {
        int bin = (int)((local_data[i] - min_meas) / bin_width);
        if (bin >= bin_count) bin = bin_count - 1; 
        local_bins[bin]++;
    }

    MPI_Reduce(local_bins, bin_counts, bin_count, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("Histograma:\n");
        for (int i = 0; i < bin_count; i++) {
            float bin_start = min_meas + i * bin_width;
            float bin_end = bin_start + bin_width;
            printf("[%.1f - %.1f): ", bin_start, bin_end);
            for (int j = 0; j < bin_counts[i]; j++) {
                printf("X");
            }
            printf(" (%d)\n", bin_counts[i]);
        }
    }

    free(local_data);
    free(local_bins);
    if (my_rank == 0) {
        free(bin_counts);
        free(sendcounts);
        free(displs);
    }

    MPI_Finalize();
    return 0;
}
