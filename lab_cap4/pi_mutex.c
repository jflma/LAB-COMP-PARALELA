#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

long long n = 100000000; // 10^8 iteraciones
int thread_count;
double sum = 0.0;
pthread_mutex_t mutex; 

void* Thread_sum(void* rank);

int main() {
    int thread_counts[] = {1, 2, 4, 8, 16, 32, 64};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    printf("Medición de tiempos (mutex) con n = %lld\n", n);
    printf("-------------------------------------------------------\n");

    for (int t = 0; t < num_tests; t++) {
        thread_count = thread_counts[t];
        pthread_t* thread_handles = malloc(thread_count * sizeof(pthread_t));

        sum = 0.0;
        pthread_mutex_init(&mutex, NULL);

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

        pthread_mutex_destroy(&mutex);
        printf("Hilos: %2d | Pi ≈ %.15f | Tiempo: %.3f s\n", thread_count, pi, tiempo);

        free(thread_handles);
    }

    printf("-------------------------------------------------------\n");
    return 0;
}

void* Thread_sum(void* rank) {
    long my_rank = (long) rank;
    double factor;
    double my_sum = 0.0;
    long long i;
    long long my_n = n / thread_count;
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;

    for (i = my_first_i; i < my_last_i; i++, factor = -factor) {
        my_sum += factor / (2 * i + 1);
    }

    pthread_mutex_lock(&mutex);
    sum += my_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}
