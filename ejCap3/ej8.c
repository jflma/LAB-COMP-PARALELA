#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int* merge(int* arr1, int n1, int* arr2, int n2) {
    int* result = malloc((n1 + n2) * sizeof(int));
    int i = 0, j = 0, k = 0;

    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j])
            result[k++] = arr1[i++];
        else
            result[k++] = arr2[j++];
    }
    while (i < n1) result[k++] = arr1[i++];
    while (j < n2) result[k++] = arr2[j++];
    return result;
}

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int n, local_n;
    int *local_keys, *all_keys = NULL;

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

    local_keys = malloc(local_n * sizeof(int));
    srand(my_rank + 1);  
    for (int i = 0; i < local_n; i++) {
        local_keys[i] = rand() % 100; 
    }

    qsort(local_keys, local_n, sizeof(int), compare);

    if (my_rank == 0) all_keys = malloc(n * sizeof(int));
    MPI_Gather(local_keys, local_n, MPI_INT,
               all_keys, local_n, MPI_INT,
               0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("Sunlistas ordenadas iniciales:\n");
        for (int i = 0; i < n; i++) {
            printf("%d ", all_keys[i]);
        }
        printf("\n\n");
    }

    // --------- Tree-structured merge ---------
    int step = 1;
    int* merged = local_keys;
    int merged_size = local_n;

    while (step < comm_sz) {
        if (my_rank % (2 * step) == 0) {
            if (my_rank + step < comm_sz) {
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, my_rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int* recv_keys = malloc(recv_size * sizeof(int));
                MPI_Recv(recv_keys, recv_size, MPI_INT, my_rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int* new_merged = merge(merged, merged_size, recv_keys, recv_size);
                free(merged);
                free(recv_keys);
                merged = new_merged;
                merged_size += recv_size;
            }
        } else {
            int dest = my_rank - step;
            MPI_Send(&merged_size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(merged, merged_size, MPI_INT, dest, 0, MPI_COMM_WORLD);
            free(merged);
            break;
        }
        step *= 2;
    }

    if (my_rank == 0) {
        printf("Lista ordenada final:\n");
        for (int i = 0; i < merged_size; i++) {
            printf("%d ", merged[i]);
        }
        printf("\n");
        free(merged);
        free(all_keys);
    }

    MPI_Finalize();
    return 0;
}
