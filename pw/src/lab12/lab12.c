#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdbool.h>

const long counter = 5000000;

long x = 0;

atomic_bool lock = false;


void critical_section(void) {
    long y;
    y = x;
    y = y + 1;
    x = y;
}

void local_section(void) {
}

void entry_protocol(int nr) {
    bool expected = false;

    //opcja compare_exchange
    while (!atomic_compare_exchange_weak(&lock, &expected, true)) {
        expected = false;
    }

    //opcja exchange
    //while (atomic_exchange(&lock, true)) {//aktywne czekanie
    //}

}

void exit_protocol(int nr) {
    atomic_store(&lock, false);
}

void* th(void* arg) {
    int nr = *(int*)arg;
    for (long i = 0; i < counter; i++) {
        local_section();
        entry_protocol(nr);
        critical_section();
        exit_protocol(nr);
    }
    return NULL;
}

void* monitor(void* arg) {
    (void)arg; // Unused argument
    long prev = 0;
    while (1) {
        prev = x;
        sleep(2);
        if (prev == x)
            printf("nothing happened\n");
        else
            printf("monitor: %ld\n", x);
    }
}

int main() {
    printf("main() starts\n");
    pthread_t monitor_th;
    pthread_t t1, t2;

    pthread_create(&monitor_th, NULL, monitor, NULL);
    pthread_create(&t1, NULL, th, (void*)&(int){0});
    pthread_create(&t2, NULL, th, (void*)&(int){1});

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(monitor_th, NULL);

    printf("main() completes: %ld\n", x);

    return 0;
}
