#ifndef CUSTOM_SEM_H
#define CUSTOM_SEM_H

typedef struct {
    int value;
} custom_sem_t;

void sem_init(custom_sem_t* sem, int initial_value);

void sem_acquire(custom_sem_t* sem, int amount);

void sem_release(custom_sem_t* sem, int amount);

#endif // CUSTOM_SEM_H
