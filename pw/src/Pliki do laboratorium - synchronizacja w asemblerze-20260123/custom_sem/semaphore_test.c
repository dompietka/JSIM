#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "custom_sem.h"


#define MTX_NUM_THREADS 5
#define MTX_NUM_ITERATIONS 100000

#define SEM_NUM_THREADS 40
#define SEM_ALLOWED_THREADS 7
#define SEM_SLEEP_TIME 100000

custom_sem_t semaphore;
pthread_barrier_t barrier;
atomic_bool error = false;

void* inc_thread(void* arg) {
    int* shared_variable = (int*) arg;
    for (int i = 0; i < MTX_NUM_ITERATIONS; i++) {
        sem_acquire(&semaphore, 1);
        (*shared_variable)++;
        sem_release(&semaphore, 1);
    }
    return NULL;
}

void* sem_thread(void* arg) {
    atomic_int* shared_variable = (atomic_int*) arg;

    pthread_barrier_wait(&barrier);

    sem_acquire(&semaphore, 1);
    atomic_fetch_add(shared_variable, 1);

    if (atomic_load(shared_variable) > SEM_ALLOWED_THREADS) {
        atomic_store(&error, true);
    }

    usleep(SEM_SLEEP_TIME);

    atomic_fetch_sub(shared_variable, 1);
    sem_release(&semaphore, 1);

    return NULL;
}

void mutex_test() {
    int shared_variable = 0;
    pthread_t threads[MTX_NUM_THREADS];

    sem_init(&semaphore, 1);

    for (int i = 0; i < MTX_NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, inc_thread, &shared_variable);
    }

    for (int i = 0; i < MTX_NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Shared variable: %d\n", shared_variable);
    assert(shared_variable == MTX_NUM_THREADS * MTX_NUM_ITERATIONS);
}

void sem_test() {
    atomic_int shared_variable = 0;
    pthread_t threads[SEM_NUM_THREADS];

    pthread_barrier_init(&barrier, NULL, SEM_NUM_THREADS);

    sem_init(&semaphore, SEM_ALLOWED_THREADS);

    for (int i = 0; i < SEM_NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, sem_thread, &shared_variable);
    }

    for (int i = 0; i < SEM_NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Shared variable: %d. Error: %s\n", atomic_load(&shared_variable), atomic_load(&error) ? "true" : "false");
    assert(atomic_load(&error) == false);
    printf("Semaphore test passed without errors.\n");
    pthread_barrier_destroy(&barrier);
}

int main() {
    mutex_test();

    sem_test();

    return 0;
}
