#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

double** A;
double* x;
double* y;
int m, n;
int thread_count;
pthread_mutex_t y_mutex;

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

void* pth_mat_vect_columns(void* rank) {
    long my_rank = (long)rank;
    int local_n = n / thread_count;
    int my_first_col = my_rank * local_n;
    int my_last_col = (my_rank == thread_count - 1) ? (n - 1) : (my_first_col + local_n - 1);

    double* local_y = calloc(m, sizeof(double));
    if (!local_y) {
        perror("Fallo al alocar local_y");
        return NULL;
    }

    for (int i = 0; i < m; i++) {
        for (int j = my_first_col; j <= my_last_col; j++) {
            local_y[i] += A[i][j] * x[j];
        }
    }

    pthread_mutex_lock(&y_mutex);
    for (int i = 0; i < m; i++) {
        y[i] += local_y[i];
    }
    pthread_mutex_unlock(&y_mutex);

    free(local_y);
    return NULL;
}

double run_benchmark(int p_m, int p_n, int p_thread_count) {
    m = p_m;
    n = p_n;
    thread_count = p_thread_count;

    A = malloc(m * sizeof(double*));
    x = malloc(n * sizeof(double));
    y = calloc(m, sizeof(double));
    for(int i = 0; i < m; i++) {
        A[i] = malloc(n * sizeof(double));
    }

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            A[i][j] = 1.0;
        }
    }
    for(int i = 0; i < n; i++) {
        x[i] = 2.0;
    }

    pthread_mutex_init(&y_mutex, NULL);
    pthread_t* thread_handles = malloc(thread_count * sizeof(pthread_t));

    double start_time = get_time();

    for (long i = 0; i < thread_count; i++) {
        pthread_create(&thread_handles[i], NULL, pth_mat_vect_columns, (void*)i);
    }

    for (long i = 0; i < thread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    double end_time = get_time();

    pthread_mutex_destroy(&y_mutex);
    free(thread_handles);
    for(int i = 0; i < m; i++) {
        free(A[i]);
    }
    free(A);
    free(x);
    free(y);

    return end_time - start_time;
}

void print_header() {
    printf("+-------+-------------------+-------------------+-------------------+\n");
    printf("| Hilos | 8,000,000 x 8     |   8,000 x 8,000   | 8 x 8,000,000     |\n");
    printf("+-------+-------------------+-------------------+-------------------+\n");
}

void print_footer() {
    printf("+-------+-------------------+-------------------+-------------------+\n");
}

int main() {
    int dims[][2] = {
        {8000000, 8},
        {8000, 8000},
        {8, 8000000}
    };
    int thread_configs[] = {1, 2, 4};
    int num_dims = 3;
    int num_threads_configs = 3;

    double results[num_threads_configs][num_dims];

    printf("Iniciando benchmarks de Multiplicación Matriz-Vector (Descomposición por Columnas)...\n");
    
    for (int i = 0; i < num_dims; i++) {
        for (int j = 0; j < num_threads_configs; j++) {
            int current_m = dims[i][0];
            int current_n = dims[i][1];
            int current_threads = thread_configs[j];
            printf("  Corriendo: %d x %d con %d hilo(s)...\n", current_m, current_n, current_threads);
            results[j][i] = run_benchmark(current_m, current_n, current_threads);
        }
    }
    printf("Benchmarks finalizados.\n\n");

    print_header();
    for (int j = 0; j < num_threads_configs; j++) {
        int threads = thread_configs[j];
        printf("|   %d   |    %-14.4fs |    %-14.4fs |    %-14.4fs |\n",
               threads, results[j][0], results[j][1], results[j][2]);
    }
    print_footer();

    return 0;
}