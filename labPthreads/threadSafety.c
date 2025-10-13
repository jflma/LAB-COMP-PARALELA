#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_TO_PRODUCE 20

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_full;
pthread_cond_t cond_empty;

void* producer(void* arg) {
    long id = (long)arg;
    for (int i = 0; i < ITEMS_TO_PRODUCE; i++) {
        int item = (id * ITEMS_TO_PRODUCE) + i;

        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            printf("Productor %ld: Búfer LLENO. Esperando...\n", id);
            pthread_cond_wait(&cond_empty, &mutex);
        }

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Productor %ld: -> Produjo %d (Total: %d)\n", id, item, count);

        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);
        
        usleep((rand() % 100) * 1000);
    }
    return NULL;
}

void* consumer(void* arg) {
    long id = (long)arg;
    int total_items = NUM_PRODUCERS * ITEMS_TO_PRODUCE;
    for (int i = 0; i < total_items / NUM_CONSUMERS; i++) {
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            printf("Consumidor %ld: Búfer VACÍO. Esperando...\n", id);
            pthread_cond_wait(&cond_full, &mutex);
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumidor %ld: <- Consumió %d (Total: %d)\n", id, item, count);

        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex);
        
        usleep((rand() % 200) * 1000);
    }
    return NULL;
}

int main() {
    pthread_t prod_threads[NUM_PRODUCERS];
    pthread_t cons_threads[NUM_CONSUMERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_full, NULL);
    pthread_cond_init(&cond_empty, NULL);

    srand(time(NULL));

    printf("Iniciando simulación Productor-Consumidor...\n");

    for (long i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&prod_threads[i], NULL, producer, (void*)i);
    }
    for (long i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&cons_threads[i], NULL, consumer, (void*)i);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(prod_threads[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_full);
    pthread_cond_destroy(&cond_empty);
    
    printf("\nSimulación finalizada. Items en búfer: %d\n", count);

    return 0;
}