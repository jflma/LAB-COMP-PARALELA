#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

long long n = 100000000;
int thread_count;
double sum = 0.0;
volatile int flag;

void* Thread_sum(void* rank);

int main() {
    int thread_counts[] = {1, 2, 4, 8, 16, 32, 64};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    printf("Medicion de tiempos con n = %lld\n", n);
    printf("--------------------------------------------------\n");

    for (int t = 0; t < num_tests; t++) {
        thread_count = thread_counts[t];
        pthread_t* thread_handles = malloc(thread_count * sizeof(pthread_t));

        sum = 0.0;
        flag = 0;

        clock_t inicio = clock();

        for (long thread = 0; thread < thread_count; thread++) {
            pthread_create(&thread_handles[thread], NULL, Thread_sum, (void*) thread);
        }

        for (long thread = 0; thread < thread_count; thread++) {
            pthread_join(thread_handles[thread], NULL);
        }

        clock_t fin = clock();
        double tiempo = (double)(fin - inicio) / CLOCKS_PER_SEC;
        double pi = 4.0 * sum;

        printf("Hilos: %2d | Pi ≈ %.15f | Tiempo: %.3f s\n", thread_count, pi, tiempo);

        free(thread_handles);
    }

    printf("--------------------------------------------------\n");
    return 0;
}

void* Thread_sum(void* rank) {
    long my_rank = (long) rank;
    double factor;
    long long i;
    long long my_n = n / thread_count;
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;

    for (i = my_first_i; i < my_last_i; i++, factor = -factor) {
        while (flag != my_rank);
        sum += factor / (2 * i + 1);
        flag = (flag + 1) % thread_count;
    }

    return NULL;
}
