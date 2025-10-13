#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h> 

#define MAX_KEY 65535 
#define INITIAL_KEYS 1000
#define TOTAL_OPS 100000

typedef struct list_node_s {
    int data;
    struct list_node_s* next;
    pthread_mutex_t mutex;
} list_node_t;

static list_node_t* head = NULL;
static int thread_count;
static int total_ops;
static double member_fraction, insert_fraction, delete_fraction;

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

void init_list() {
    head = malloc(sizeof(list_node_t));
    head->data = -1; 
    pthread_mutex_init(&head->mutex, NULL);

    list_node_t* tail = malloc(sizeof(list_node_t));
    tail->data = MAX_KEY + 1; 
    tail->next = NULL;
    pthread_mutex_init(&tail->mutex, NULL);
    
    head->next = tail;
}

void destroy_list() {
    list_node_t* curr = head;
    while (curr) {
        list_node_t* next = curr->next;
        pthread_mutex_destroy(&curr->mutex);
        free(curr);
        curr = next;
    }
}

int Member(int value) {
    list_node_t *pred, *curr;
    
    pthread_mutex_lock(&head->mutex);
    pred = head;
    curr = pred->next;
    pthread_mutex_lock(&curr->mutex);

    while (curr->data < value) {
        pthread_mutex_unlock(&pred->mutex);
        pred = curr;
        curr = curr->next;
        if (curr) {
            pthread_mutex_lock(&curr->mutex);
        } else {
            pthread_mutex_unlock(&pred->mutex);
            return 0;
        }
    }
    
    int found = (curr->data == value);
    pthread_mutex_unlock(&pred->mutex);
    pthread_mutex_unlock(&curr->mutex);
    return found;
}

int Insert(int value) {
    list_node_t *pred, *curr;
    
    pthread_mutex_lock(&head->mutex);
    pred = head;
    curr = pred->next;
    pthread_mutex_lock(&curr->mutex);
    
    while (curr->data < value) {
        pthread_mutex_unlock(&pred->mutex);
        pred = curr;
        curr = curr->next;
        if (curr) pthread_mutex_lock(&curr->mutex);
        else { 
             pthread_mutex_unlock(&pred->mutex);
             return 0;
        }
    }
    
    if (curr->data == value) { 
        pthread_mutex_unlock(&pred->mutex);
        pthread_mutex_unlock(&curr->mutex);
        return 0; 
    }
    
    list_node_t* new_node = malloc(sizeof(list_node_t));
    new_node->data = value;
    new_node->next = curr;
    pthread_mutex_init(&new_node->mutex, NULL);
    pred->next = new_node;
    
    pthread_mutex_unlock(&pred->mutex);
    pthread_mutex_unlock(&curr->mutex);
    return 1;
}

int Delete(int value) {
    list_node_t *pred, *curr;

    pthread_mutex_lock(&head->mutex);
    pred = head;
    curr = pred->next;
    pthread_mutex_lock(&curr->mutex);

    while (curr->data < value) {
        pthread_mutex_unlock(&pred->mutex);
        pred = curr;
        curr = curr->next;
        if (curr) pthread_mutex_lock(&curr->mutex);
        else {
            pthread_mutex_unlock(&pred->mutex);
            return 0;
        }
    }

    if (curr->data != value) { 
        pthread_mutex_unlock(&pred->mutex);
        pthread_mutex_unlock(&curr->mutex);
        return 0;
    }
    
    pred->next = curr->next;
    pthread_mutex_unlock(&pred->mutex);
    pthread_mutex_unlock(&curr->mutex);
    
    pthread_mutex_destroy(&curr->mutex);
    free(curr);
    return 1;
}

void* thread_work(void* rank) {
    long my_rank = (long)rank;
    int ops_per_thread = total_ops / thread_count;
    unsigned int seed = time(NULL) + my_rank;

    for (int i = 0; i < ops_per_thread; i++) {
        double op_selector = (double)rand_r(&seed) / RAND_MAX;
        int key = rand_r(&seed) % MAX_KEY;

        if (op_selector < member_fraction) {
            Member(key);
        } else if (op_selector < member_fraction + insert_fraction) {
            Insert(key);
        } else {
            Delete(key);
        }
    }
    return NULL;
}


double run_benchmark(int p_thread_count, double p_member_frac, double p_insert_frac) {
    thread_count = p_thread_count;
    member_fraction = p_member_frac;
    insert_fraction = p_insert_frac;
    delete_fraction = 1.0 - member_fraction - insert_fraction;
    total_ops = TOTAL_OPS;

    init_list();
    unsigned int seed = time(NULL);
    int i = 0;
    while(i < INITIAL_KEYS) {
        if (Insert(rand_r(&seed) % MAX_KEY)) {
            i++;
        }
    }
    
    pthread_t* thread_handles = malloc(thread_count * sizeof(pthread_t));
    
    double start_time = get_time();
    
    for (long i = 0; i < thread_count; i++) {
        pthread_create(&thread_handles[i], NULL, thread_work, (void*)i);
    }
    
    for (long i = 0; i < thread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }
    
    double end_time = get_time();
    
    free(thread_handles);
    destroy_list();
    
    return end_time - start_time;
}

void print_table_header() {
    printf("+--------------------------+----------+----------+----------+----------+\n");
    printf("|       Implementacion     | 1 Hilo   | 2 Hilos  | 4 Hilos  | 8 Hilos  |\n");
    printf("+--------------------------+----------+----------+----------+----------+\n");
}

void print_table_row(const char* title, double times[]) {
    printf("| %-24s | %-8.4fs | %-8.4fs | %-8.4fs | %-8.4fs |\n", title, times[0], times[1], times[2], times[3]);
}

void print_table_footer() {
     printf("+--------------------------+----------+----------+----------+----------+\n\n");
}


int main() {
    int thread_counts[] = {1, 2, 4, 8};
    int num_thread_configs = sizeof(thread_counts) / sizeof(int);
    double results_table1[num_thread_configs];
    double results_table2[num_thread_configs];

    printf("Ejecutando Benchmark 1 (99.9%% Member, 0.05%% Insert, 0.05%% Delete)...\n");
    for (int i = 0; i < num_thread_configs; i++) {
        printf("  Corriendo con %d hilo(s)...\n", thread_counts[i]);
        results_table1[i] = run_benchmark(thread_counts[i], 0.999, 0.0005);
    }
    printf("Benchmark 1 finalizado.\n\n");

    printf("Ejecutando Benchmark 2 (80%% Member, 10%% Insert, 10%% Delete)...\n");
    for (int i = 0; i < num_thread_configs; i++) {
        printf("  Corriendo con %d hilo(s)...\n", thread_counts[i]);
        results_table2[i] = run_benchmark(thread_counts[i], 0.80, 0.10);
    }
    printf("Benchmark 2 finalizado.\n\n");
    
    printf("======================================================================\n");

    printf("Tabla 1: 1000 Claves Iniciales, 100,000 ops (99.9%% Member, 0.05%% Ins, 0.05%% Del)\n");
    print_table_header();
    print_table_row("Hand-over-Hand Locking", results_table1);
    print_table_footer();

    printf("Tabla 2: 1000 Claves Iniciales, 100,000 ops (80%% Member, 10%% Ins, 10%% Del)\n");
    print_table_header();
    print_table_row("Hand-over-Hand Locking", results_table2);
    print_table_footer();

    return 0;
}