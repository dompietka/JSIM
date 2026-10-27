#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "../common/plant.h"
#include "../common/err.h"


typedef enum { //makro do stanow zadania 
    WAITING, // = 0
    RUNNING, // = 1
    DONE,    // = 2
    FAILED   // = 3
} task_state_t;

typedef struct {
    task_t *task;              
    task_state_t state;
    int stan; //ktore stanowisko zajete
    int finished_workers;
    pthread_cond_t done_cond;
    int sbd_waiting;
} task_internal_t;

typedef struct task_node {
    task_internal_t task;
    struct task_node *next;
} task_node_t;

typedef struct {
    worker_t *worker;
    pthread_cond_t wait_for_task;
    int busy;
    int go_home;
    task_internal_t* assigned_task;
    int nr;
} worker_state_t;

typedef struct { //fabryka ze stanami, jedna na caly program
    int exists;
    int destroyed;  
    
    int S;
    int *stations;  
    int available_stations;
    int* stations_busy;  
    int max_stattion_capacity;

    int P;
    //tablica moze byc stalego rozmiaru, bo znamy max liczbe pracownikow
    worker_state_t* workers;
    pthread_t *thread_ids;
    int added_workers;
    int gone_workers;

    //lista na zadania
    task_node_t *head;      
    task_node_t *tail;      

    pthread_t boss_thread;
    pthread_cond_t boss_office;
    pthread_mutex_t mutex;
} plant_t;

static plant_t plant;

void set_timeout(struct timespec *ts, int seconds) {
    ts->tv_sec = time(0) + seconds;
    ts->tv_nsec = 0; 
    return;
}

void plant_push_task(plant_t *plant, task_t* data) { //zakladam ze mam mutex
    task_node_t *new_node = malloc(sizeof(task_node_t));

    // Inicjalizujemy bezpośrednio wewnątrz węzła
    new_node->task.task = data;
    new_node->task.state = WAITING;
    new_node->task.finished_workers = 0;
    new_node->task.sbd_waiting = 0;
    ASSERT_ZERO(pthread_cond_init(&new_node->task.done_cond, NULL));
    new_node->next = NULL;

    if (plant->tail == NULL) {
        plant->head = new_node;
        plant->tail = new_node;
    } else {
        plant->tail->next = new_node;
        plant->tail = new_node;
    }
}

void* boss_function(void* arg)
{
    //dostaje swoj watek juz w init, join bedzie w destroy
    ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));
    while (!plant.destroyed || plant.head != NULL) {
        //fprintf(stderr, "Boss sprawdza zadania...\n");
        //jesli nie ma zadan albo brak stanowisk to czekam
        if (plant.head != NULL && plant.available_stations > 0) { 
            //fprintf(stderr, "Boss szuka zadan do wykonania...\n");
            //fprintf(stderr, "Aktualny czas: %ld\n", time(0));
            time_t curr_time = time(0); 
            int maks_capacity = 0;
            int avaliable_workers = 0;
            //maks wolne stanowisko
            for (int i=0;i<plant.S;i++){
                if (!plant.stations_busy[i] && plant.stations[i] > maks_capacity){
                    maks_capacity = plant.stations[i];
                }               
            }
            //fprintf(stderr, "Maksymalna dostepna pojemnosc stanowiska: %d\n", maks_capacity);
            //fprintf(stderr, "Liczba dostepnych stanowisk: %d\n", plant.available_stations);

            //licze wolnych pracownikow
            //jak spi w pracy a powinien w domu to odsylam do domu
            //fprintf(stderr, "Sprawdzam dostepnych pracownikow...\n");
            for (int i=0;i<plant.added_workers;i++){
                if (!plant.workers[i].busy &&  plant.workers[i].worker->start <= curr_time ){
                    if (plant.workers[i].worker->end < curr_time){
                    plant.gone_workers++;
                    ASSERT_ZERO(pthread_cond_signal(&plant.workers[i].wait_for_task)); 
                  }
                    else
                    avaliable_workers++;
                }
            }
            //fprintf(stderr, "Liczba dostepnych pracownikow: %d\n", avaliable_workers);

            //szukam zadania ktore moge zrealizowac przy obecnych zasobach
            //przy okazji oznaczam te ktorych nie dam rady zrealizowac i budze odbiorce
            task_node_t* current_task_node = plant.head;
            task_node_t* prev_task_node = NULL;
            while (current_task_node != NULL) {
                //fprintf(stderr, "Analizuje zadanie %d...\n", current_task_node->task.task->id);
                //jesli juz wiem ze go nie skoncze
                if ((current_task_node->task.state!=FAILED)&&(current_task_node->task.task->capacity > plant.P - plant.gone_workers 
                    || current_task_node->task.task->capacity > plant.max_stattion_capacity)) {
                    current_task_node->task.state = FAILED;
                    //fprintf(stderr, "Boss odrzuca zadanie %d z powodu braku zasobow.\n", current_task_node->task.task->id);
                    ASSERT_ZERO(pthread_cond_signal(&current_task_node->task.done_cond));
                }
                if (plant.destroyed && (current_task_node->task.state == FAILED || current_task_node->task.state == DONE) && current_task_node->task.sbd_waiting == 0) {
                    //usun zadanie z listy
                    //fprintf(stderr, "Boss usuwa zadanie %d z listy zadan bo nikt go nie kocha\n", current_task_node->task.task->id);
                   task_node_t* to_delete = current_task_node;
                    current_task_node = current_task_node->next; // Przesuwamy wskaźnik pętli DALEJ zanim usuniemy

                    if (prev_task_node == NULL) {
                        plant.head = current_task_node;
                    } else {
                        prev_task_node->next = current_task_node;
                    }
                    if (to_delete == plant.tail) {
                        plant.tail = prev_task_node;
                    }

                    pthread_cond_destroy(&to_delete->task.done_cond);
                    free(to_delete);
                    continue;
                } 
                /*//fprintf(stderr, "Sprawdzam zadanie %d: stan %d, start %ld, pojemnosc %d\n",
                        current_task_node->task.task->id,
                        current_task_node->task.state,
                        current_task_node->task.task->start,
                        current_task_node->task.task->capacity);*/
                if (current_task_node->task.state == WAITING &&
                    current_task_node->task.task->start <= curr_time &&
                    current_task_node->task.task->capacity <= maks_capacity &&
                    current_task_node->task.task->capacity <= avaliable_workers) {

                        //fprintf(stderr, "Boss przydziela zadanie %d do realizacji.\n", current_task_node->task.task->id);
                    //rezerwuje stanowisko
                    for (int i = 0; i < plant.S; i++) {
                        if (!plant.stations_busy[i] && plant.stations[i] >= current_task_node->task.task->capacity) {
                            plant.stations_busy[i] = 1;
                            plant.available_stations--;
                            current_task_node->task.stan = i;
                            break;
                        }
                    }
                    //fprintf(stderr, "Zadanie %d przydzielone do stanowiska %d.\n", current_task_node->task.task->id, current_task_node->task.stan);

                    //przydzielam zadanie
                    int capacity = current_task_node->task.task->capacity;
                    for (int i = 0; i < plant.added_workers && capacity > 0; i++) {
                        if (!plant.workers[i].busy && plant.workers[i].worker->start <= curr_time &&
                            plant.workers[i].worker->end >= curr_time) {
                            plant.workers[i].busy = 1;
                            plant.workers[i].assigned_task = &current_task_node->task;
                            plant.workers[i].nr = current_task_node->task.task->capacity - capacity;
                            capacity--;
                            ASSERT_ZERO(pthread_cond_signal(&plant.workers[i].wait_for_task));
                            ////fprintf(stderr, "Pracownik %d przydzielony do zadania %d.\n", plant.workers[i].worker->id, current_task_node->task.task->id);
                        }
                    }
                    current_task_node->task.state = RUNNING;
                    break; //wracam do glownej petli zeby grzecznie poczekac 
                }
                prev_task_node = current_task_node;
                current_task_node = current_task_node->next;
                
            }
        }
        
            struct timespec timeout;
            set_timeout(&timeout, 1);
            //fprintf(stderr, "Boss czeka na sygnal lub timeout...\n");
            int res = pthread_cond_timedwait(&plant.boss_office, &plant.mutex, &timeout);
            if (res!=0 && res!=110){ //110 to ETIMEDOUT
                //fprintf(stderr, "Boss: blad podczas czekania na sygnal. Kod bledu: %d\n", res);
                return NULL;
            }
        
          
    }
    //fprintf(stderr, "Boss konczy dzialalnosc.\n");
    ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
    return NULL;
}

int init_plant(int* stations, int n_stations, int n_workers)
{

    //fprintf(stderr,"Inicjalizacja fabryki z %d stanowiskami i %d pracownikami.\n", n_stations, n_workers);
    plant.exists = 1;
    plant.destroyed = 0;

    plant.S = n_stations;
    plant.stations = stations;
    plant.max_stattion_capacity = 0;
    for (int i=0;i<n_stations;i++){
        if (stations[i] > plant.max_stattion_capacity){
            plant.max_stattion_capacity = stations[i];
        }
    }
    plant.available_stations = n_stations;
    plant.stations_busy = malloc(n_stations * sizeof(int));
    for (int i = 0; i < n_stations; i++) {
        plant.stations_busy[i] = 0;
    }

    plant.P = n_workers;
    plant.workers = malloc(n_workers * sizeof(worker_state_t));
    plant.thread_ids = malloc(n_workers * sizeof(pthread_t));
    plant.added_workers = 0; 
    plant.gone_workers = 0;
    
    plant.head = NULL;
    plant.tail = NULL;

    ASSERT_ZERO(pthread_mutex_init(&plant.mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&plant.boss_office, NULL));
    ASSERT_ZERO(pthread_create(&plant.boss_thread, NULL, boss_function, NULL));

    //fprintf(stderr, "Fabryka zainicjalizowana pomyslnie.\n");
    return PLANTOK;
}

void* worker_routine(void*arg) {
   
    //zeby był ok typ
    worker_state_t* w = (worker_state_t*)arg;
    while (1) {
        ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));

        //przestane czekac jak dostane zadanie
        //albo bedzie czas do domu
        while (!w->busy && !w->go_home && w->worker->end > time(0)) {
            ASSERT_ZERO(pthread_cond_wait(&w->wait_for_task, &plant.mutex));
        }

        if (w->busy){ //nawet jeśli po czasie do domu
            task_internal_t* t = w->assigned_task;
            int nr = w->nr;
            ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
            //fprintf(stderr, "Pracownik %d wykonuje zadanie %d (indeks %d)...\n", w->worker->id, t->task->id, nr);
           //to powinno byc thread safe bo nr
            t->task->results[nr] = w->worker->work(w->worker, t->task, nr);
            ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));
            w->busy = 0;
            t->finished_workers++;
            //jesli jestem ostatnim to zmieniam stan zadania
            //zwlaniam stanowisko
            //budze potencjalnego odbiorce
            if( t->finished_workers == t->task->capacity){
                t->state = DONE;
                plant.stations_busy[t->stan] = 0;
                plant.available_stations++;
                ASSERT_ZERO(pthread_cond_signal(&t->done_cond));
                ASSERT_ZERO(pthread_cond_signal(&plant.boss_office));
            }
            ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
            //fprintf(stderr, "Pracownik %d zakonczyl zadanie %d.\n", w->worker->id, t->task->id);
        } else {
            ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
            break;
        }
    }
    //umieram
    //fprintf(stderr, "Pracownik %d umarl.\n", w->worker->id);
    return NULL;
}

int add_worker(worker_t* w)
{
    ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));

    //jesli nie mam fabryki to parcownik nie moze byc dodany
    if (plant.destroyed || !plant.exists) { 
        ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
        return ERROR;
    }

    //zwiekszam licznik, i deklaruje wszystkie potrzebne pola
    //pamiec zostala zadeklarowana w init_plant
    plant.workers[plant.added_workers].worker = w;
    plant.workers[plant.added_workers].busy = 0;
    plant.workers[plant.added_workers].go_home = 0;
    plant.workers[plant.added_workers].assigned_task = NULL;
    ASSERT_ZERO(pthread_cond_init(&plant.workers[plant.added_workers].wait_for_task, NULL));
    //tworze watek pracownika w tablicy juz
    ASSERT_ZERO(pthread_create(&plant.thread_ids[plant.added_workers], NULL, worker_routine, &plant.workers[plant.added_workers]));
    ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
    plant.added_workers++;
    //fprintf(stderr, "Dodano pracownika %d do fabryki.\n", w->id);
    return PLANTOK;
}

int add_task(task_t* t)
{
    ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));
    // zniszczona       || nie istnieje  ||  za duze wymagania
    if (plant.destroyed || !plant.exists || t->capacity > plant.P) {
        ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
        return -1;
    }

    plant_push_task(&plant, t); 
    //fprintf(stderr, "Dodano zadanie %d do kolejki.\n", t->id);
    //budze szefa, zeby sprawdzil czy moze cos zrobic
    ASSERT_ZERO(pthread_cond_signal(&plant.boss_office));
    ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));

    return PLANTOK;
}

int collect_task(task_t* t)
{
    ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));

    if (plant.destroyed || !plant.exists) {
        ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
        return ERROR;
    }

    task_node_t *current = plant.head;
    task_node_t *prev = NULL;
   while (current != NULL) {
        if (current->task.task == t) { 
            while (current->task.state != DONE && current->task.state != FAILED) {
                current->task.sbd_waiting++;
                //fprintf(stderr, "Czekam na zadanie %d\n", t->id);
                ASSERT_ZERO(pthread_cond_wait(&current->task.done_cond, &plant.mutex));
            }

            int final_status = (current->task.state == DONE) ? PLANTOK : ERROR;
            //usuwamy z listy
            //fprintf(stderr, "Doczekałem się zadania %d, stan fabryki to %d\n", t->id, plant.destroyed);

            if (prev == NULL) { //poczatek
                plant.head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == plant.tail) { //jesli byl ostatni
                plant.tail = prev;
            }
            ASSERT_ZERO(pthread_cond_destroy(&current->task.done_cond));
            free(current);

            ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
            ////fprintf(stderr, "Wynik zadania %d zebrany, ten wynik to %d\n", t->id, t->results[0]);
            return final_status;
        }

        prev = current;
        current = current->next;
    }

    ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
    return ERROR;
}

int destroy_plant()
{
    //fprintf(stderr, "Rozpoczynam niszczenie fabryki...\n");
    ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));
    plant.destroyed = 1;
    ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
    //boss juz wie ze czas do domu, wiec czekam az zrobi to co musi
    pthread_join(plant.boss_thread, NULL);

    //fprintf(stderr, "Boss zakonczyl dzialalnosc, zamykam pracownikow...\n");

    //bez mutexa bo jest destroyed wiec nikt nie doda pracownika    
    for (int i = 0; i < plant.added_workers; i++) {
        ASSERT_ZERO(pthread_mutex_lock(&plant.mutex));
        plant.workers[i].go_home = 1;
        ASSERT_ZERO(pthread_cond_signal(&plant.workers[i].wait_for_task));
        ASSERT_ZERO(pthread_mutex_unlock(&plant.mutex));
        pthread_join(plant.thread_ids[i], NULL);
        ASSERT_ZERO(pthread_cond_destroy(&plant.workers[i].wait_for_task));
    }

    //fprintf(stderr, "Wszyscy pracownicy zakonczyli dzialalnosc.\n");

    ASSERT_ZERO(pthread_cond_destroy(&plant.boss_office));
    free(plant.stations_busy);
    free(plant.workers);
    free(plant.thread_ids);

    ASSERT_ZERO(pthread_mutex_destroy(&plant.mutex));
    //fprintf(stderr, "Fabryka zniszczona pomyslnie.\n");
    return PLANTOK;
}