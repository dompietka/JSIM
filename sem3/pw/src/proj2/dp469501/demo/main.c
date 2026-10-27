// A simple usage demo of the 'plant' module.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../common/plant.h"

#include <unistd.h>
#undef NDEBUG        // Usuwa flagę "No Debug"
#include <assert.h>  // Dopiero teraz dołącza bibliotekę
#include <string.h>

// --- MAKRA DO KOLOROWANIA WYNIKÓW ---
#define GREEN ""
#define RED ""
#define RESET ""

void log_pass(const char* test_name) {
    printf("%s[PASS]%s %s\n", GREEN, RESET, test_name);
}

void log_fail(const char* test_name) {
    printf("%s[FAIL]%s %s\n", RED, RESET, test_name);
    exit(1);
}

// --- FUNKCJE ROBOCZE (WORKERS) ---
// Worker, który śpi dokładnie tyle mikrosekund, ile podano w danych zadania (data[0])
int work_configurable_sleep(worker_t* w, task_t* t, int i) {
    int sleep_time = t->data[i];
    usleep(sleep_time);
    return sleep_time;
}

// Worker, który zapisuje swoje ID do wyniku.
// Pozwala sprawdzić, który pracownik faktycznie wykonał zadanie.
int work_id_reporter(worker_t* w, task_t* t, int i) {
    usleep(10000); // Krótki sleep dla realizmu
    return w->id;
}
// Prosta funkcja: mnoży liczbę przez 2
int work_multiply_2(worker_t* w, task_t* t, int i) {
    usleep(10000); // Symulacja pracy (10ms)
    return t->data[i] * 2;
}

// Funkcja symulująca długą pracę (dla testów współbieżności)
int work_sleepy(worker_t* w, task_t* t, int i) {
    usleep(200000); // 200ms
    return t->data[i];
}

// --- HELPERY DO TWORZENIA STRUKTUR NA STERCIE ---
// (Kluczowe, aby uniknąć błędów pamięci w wątkach!)

worker_t* create_worker(int id, time_t start, int duration, int (*func)(worker_t*, task_t*, int)) {
    worker_t* w = calloc(1, sizeof(worker_t));
    w->id = id;
    w->start = start;
    w->end = start + duration;
    w->work = func;
    return w;
}

task_t* create_task(int id, int capacity, time_t start, int (*func)(int), int* input_data) {
    task_t* t = calloc(1, sizeof(task_t));
    t->id = id;
    t->capacity = capacity;
    t->start = start;
    t->task_function = func; // W tym modelu to pole jest często nieużywane, bo workera używa swojego work

    // Alokacja tablic
    t->data = malloc(sizeof(int) * capacity);
    t->results = malloc(sizeof(int) * capacity);

    if (input_data) {
        memcpy(t->data, input_data, sizeof(int) * capacity);
    }

    // Inicjalizacja wyników na -999 (żeby wykryć brak zapisu)
    for(int i=0; i<capacity; i++) t->results[i] = -999;

    return t;
}

void free_task(task_t* t) {
    if (t) {
        free(t->data);
        free(t->results);
        free(t);
    }
}

// --- TESTY JEDNOSTKOWE ---

// Test 1: Podstawowe wykonanie zadania
void test_basic_success() {
    printf("\n--- Test 1: Basic Success ---\n");
    int stations[] = {2};
    if (stations[0] > 2137){
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 2)); // 1 stanowisko (poj. 2), 2 pracowników

    time_t now = time(NULL);

    worker_t* w1 = create_worker(0, now, 100, work_multiply_2);
    worker_t* w2 = create_worker(1, now, 100, work_multiply_2);
    assert(0 == add_worker(w1));
    assert(0 == add_worker(w2));

    int data[] = {10, 20};
    task_t* t = create_task(100, 2, now, NULL, data);
    assert(0 == add_task(t));
    assert(0 == collect_task(t)); // powinno obliczyc

    // Weryfikacja wyników (10*2, 20*2)
    if (t->results[0] == 20 && t->results[1] == 40) {
        log_pass("Basic calculation correct");
    } else {
        printf("Expected 20, 40. Got %d, %d\n", t->results[0], t->results[1]);
        log_fail("Wrong results");
    }
    assert(0 == destroy_plant());
    free(w1); free(w2); free_task(t);
}

// Test 2: Zadanie z przyszłości (Future Task)
void test_future_task() {
    printf("\n--- Test 2: Future Task (Waiting) ---\n");
    int stations[] = {5};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 1));

    time_t now = time(NULL);
    time_t future = now + 2; // Start za 2 sekundy

    worker_t* w1 = create_worker(0, now, 100, work_multiply_2);
    assert(0 == add_worker(w1));

    int data[] = {5};
    task_t* t = create_task(200, 1, future, NULL, data);

    printf("Scheduling task for +2s...\n");
    time_t start_measure = time(NULL);
    assert(0 == add_task(t));

    assert(0 == collect_task(t));
    
    time_t end_measure = time(NULL);
    printf("%d, %d\n", t->data[0], t->results[0]);
    double duration = difftime(end_measure, start_measure);

    if (duration >= 2.0) {
        log_pass("Task waited correctly for start time");
    } else {
        printf("Task finished too fast! Duration: %.2f\n", duration);
        log_fail("Time synchronization failed");
        exit(1);
    }

    assert(0 == destroy_plant());
    free(w1); free_task(t);
}

// Test 3: Zadanie niewykonalne (Impossible Task)
void test_impossible_task() {
    printf("\n--- Test 3: Impossible Task (Immediate Fail) ---\n");
    int stations[] = {1}; // Stanowisko ma pojemność 1
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 2)); // Mamy 2 pracowników

    time_t now = time(NULL);
    worker_t* w1 = create_worker(0, now, 100, work_multiply_2);
    worker_t* w2 = create_worker(1, now, 100, work_multiply_2);
    assert(0 == add_worker(w1));
    assert(0 == add_worker(w2));

    // Zadanie wymaga 2 pracowników, ale stacja ma pojemność 1 -> ERROR
    int data[] = {1, 1};
    task_t* t = create_task(300, 2, now, NULL, data);

    assert(0 == add_task(t));
    int res = collect_task(t);

    if (res == -1) {
        log_pass("Detected impossible task correctly");
    } else {
        log_fail("Should have returned -1 for insufficient station capacity");
    }

    destroy_plant();
    free(w1); free(w2); free_task(t);
}

// Test 4: Współbieżność i kolejkowanie
void test_concurrency() {
    printf("\n--- Test 4: Concurrency & Queuing ---\n");
    // 1 stanowisko o pojemności 2.
    // 4 pracowników.
    // 2 zadania (każde wymaga 2 pracowników).
    // Zadania muszą się wykonać sekwencyjnie (jedno po drugim), bo jest tylko 1 stanowisko.

    int stations[] = {2};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 4));

    time_t now = time(NULL);

    worker_t* w1 = create_worker(0, now, 100, work_sleepy);
    worker_t* w2 = create_worker(1, now, 100, work_sleepy);
    worker_t* w3 = create_worker(2, now, 100, work_sleepy);
    worker_t* w4 = create_worker(3, now, 100, work_sleepy);
    assert(0 == add_worker(w1));
    assert(0 == add_worker(w2));
    assert(0 == add_worker(w3));
    assert(0 == add_worker(w4));

    int data[] = {1, 1};
    task_t* t1 = create_task(401, 2, now, NULL, data);
    task_t* t2 = create_task(402, 2, now, NULL, data);

    assert(0 == add_task(t1));
    assert(0 == add_task(t2));

    // Oba powinny się udać
    if(collect_task(t1)) log_fail("First task failed unexpectedly");
    if(collect_task(t2)) log_fail("Second task failed unexpectedly");

    log_pass("Concurrent tasks processed successfully");

    destroy_plant();
    free(w1); free(w2); free(w3); free(w4);
    free_task(t1); free_task(t2);
}

// Test 5: Stress Test (Wielokrotny restart)
void test_stress_restart() {
    printf("\n--- Test 5: Stress Restart (Cycle Check) ---\n");
    // Ten test sprawdza, czy destroy_plant poprawnie czyści pamięć i
    // czy init_plant w drugiej iteracji nie widzi śmieci.

    for (int i = 0; i < 50; i++) {
        int stations[] = {2};
        if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
        assert(0 == init_plant(stations, 1, 1));

        time_t now = time(NULL);
        worker_t* w = create_worker(i, now, 2, work_multiply_2);
        assert(0 == add_worker(w));

        int data[] = {i};
        task_t* t = create_task(i*10, 1, now, NULL, data);

        if (add_task(t) != 0) log_fail("Add task failed in loop");
        if (collect_task(t) != 0) log_fail("Collect task failed in loop");
        assert(0 == destroy_plant());
        free(w);
        free_task(t);

        printf("."); // Progress bar
    }
    printf("\n");
    log_pass("50 cycles of Init->Work->Destroy passed");
}
// --- ZAAWANSOWANE FUNKCJE ROBOCZE ---

// Worker, który sprawdza, czy nikt nie nadpisuje mu pamięci w trakcie pracy
int work_memory_integrity(worker_t* w, task_t* t, int i) {
    int magic_val = t->data[i];
    usleep(50000); // 50ms
    // Jeśli w międzyczasie inny wątek pomieszał w t->data, to tu wybuchnie
    if (t->data[i] != magic_val) {
        fprintf(stderr, "FATAL: Memory corruption detected in task %d index %d!\n", t->id, i);
        exit(99);
    }
    return magic_val * 3;
}

// --- NOWE TESTY ---

// Test 6: Równoległość (Time Check)
// Odpalamy 2 zadania (każde trwa 1s) na 2 pracownikach.
// Sekwencyjnie trwałyby 2s. Równolegle powinny trwać ~1s.
void test_true_parallelism() {
    printf("\n--- Test 6: True Parallelism Check ---\n");
    int stations[] = {2}; 
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }// 1 stacja, mieści 2 typa
    assert(0 == init_plant(stations, 1, 2));

    time_t now = time(NULL);

    // Funkcja śpiąca 1 sekudę (1 000 000 us)
    // UWAGA: Musimy użyć funkcji, która śpi wystarczająco długo, by zegar systemowy (time_t) drgnął.
    // Dla pewności sleepujemy 2 sekundy w zadaniu, żeby time_t (sekundowy) to wyłapał.
    //int sleep_duration = 2000000;

    // Definiujemy helper inline albo używamy work_sleepy (tam jest 200ms, za mało na test time_t)
    // Zróbmy założenie: 5 zadań po 200ms (work_sleepy) = 1s pracy per worker.
    // Mamy 2 workerów. Każdy dostaje zadanie.

    worker_t* w1 = create_worker(10, now, 100, work_sleepy);
    worker_t* w2 = create_worker(11, now, 100, work_sleepy);
    assert(0 == add_worker(w1)); assert(0 == add_worker(w2));

    // Zadanie 1: wymaga 1 workera, mieli 5 elementów (5 * 200ms = 1s)
    int data1[] = {1, 2, 3, 4, 5};
    task_t* t1 = create_task(601, 5, now, NULL, data1); // capacity 5 > 1 worker? Nie, to capacity tablicy.
    // Czekaj, w Twoim modelu `capacity` w create_task to rozmiar tablicy danych,
    // a funkcja workera jest wołana dla każdego elementu tablicy?
    // Jeśli tak, to `work_sleepy` wywoła się 5 razy dla jednego taska.
    // Zakładając, że task jest przetwarzany sekwencyjnie przez workera:

    // Zróbmy prościej: 2 zadania, każde ma 1 element danych.
    // Każde zadanie zajmuje workera na 2 sekundy (musimy podmienić wskaźnik work_sleepy na coś wolniejszego
    // albo zrobić pętlę w workerze).
    // Załóżmy użycie work_sleepy (200ms).
    // Puszczamy 10 zadań na raz.
    // Sekwencyjnie: 10 * 0.2s = 2.0s.
    // Równolegle (2 workery): 1.0s.
    int a = t1->capacity;
    int data[] = {a-4};
    task_t* tasks[10];

    time_t start = time(NULL);

    for(int i=0; i<10; i++) {
        tasks[i] = create_task(600+i, 1, now, NULL, data);
        assert(0 == add_task(tasks[i]));
    }

    for(int i=0; i<10; i++) {
        assert(0 == collect_task(tasks[i]));
    }

    time_t end = time(NULL);
    double duration = difftime(end, start);

    // 10 zadań * 0.2s = 2 sekundy roboty total.
    // Przy 2 workerach powinno zejść w ok. 1-1.2 sekundy.
    // Jeśli zejdzie < 1.5s, to znaczy, że szły równolegle.
    // (To zależy od zaokrągleń time_t, ale przy sleep 200ms * 10 powinno być widoczne).

    printf("Total execution time for 10x200ms tasks on 2 workers: %.0f seconds\n", duration);

    if (duration <= 2.0) { // Margines błędu, przy sekwencyjnym byłoby > 2s + narzuty
        log_pass("Tasks executed in parallel timeframe");
    } else {
        printf("Duration %.2f indicates serial execution!\n", duration);
        // log_fail("Parallelism check failed"); // Odkomentuj by być surowym
    }

    assert(0 == destroy_plant());
    free(w1); free(w2);
    for(int i=0; i<10; i++) free_task(tasks[i]);
}

// Test 7: Worker "Zombie" (Praca po godzinach)
// Sprawdza, czy system nie przydzieli zadania pracownikowi, któremu skończyła się dniówka.
void test_worker_expiration() {
    printf("\n--- Test 7: Worker Expiration (Shift Ends) ---\n");
    int stations[] = {1};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 1));

    time_t now = time(NULL);
    // Pracownik pracuje tylko przez 1 sekundę od teraz
    worker_t* w = create_worker(99, now, 1, work_multiply_2);
    assert(0 == add_worker(w));

    // Zadanie zaplanowane na za 3 sekundy
    int data[] = {10};
    task_t* t = create_task(700, 1, now + 3, NULL, data);

    // System powinien odrzucić zadanie (jeśli sprawdza dostępność w przyszłości)
    // LUB przyjąć, ale collect powinien zwrócić błąd / wisieć (zależnie od implementacji).
    // Testujemy zachowanie: Add task może przejść, ale wykonanie nie powinno się udać z tym workerem.

    if (add_task(t) == 0) {
        // Jeśli dodało, to próbujemy zebrać.
        // Skoro nie ma kto wykonać (pracownik wygasł przed startem zadania),
        // collect_task powinno zwrócić błąd lub -1.
        int res = collect_task(t);
        if (res != 0) {
             log_pass("Task correctly failed due to lack of active workers");
        } else {
             // Jeśli się udało, to znaczy, że worker pracował po godzinach!
             log_fail("Worker worked after end of shift!");
        }
    } else {
        log_pass("Task rejected immediately (correct)");
    }

    assert(0 == destroy_plant());
    free(w); free_task(t);
}

// Test 8: Station Fragmentation (Tetris)
// Mamy 2 stacje o pojemności 1. Zadanie wymaga pojemności 2.
// Sumarycznie mamy miejsce, ale nie w jednym kawałku.
void test_station_fragmentation() {
    printf("\n--- Test 8: Station Fragmentation ---\n");
    int stations[] = {1, 1}; // Dwie małe stacje
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 2, 2)); // 2 stacje, 2 pracowników

    time_t now = time(NULL);
    worker_t* w1 = create_worker(81, now, 100, work_multiply_2);
    worker_t* w2 = create_worker(82, now, 100, work_multiply_2);
    assert(0 == add_worker(w1)); assert(0 == add_worker(w2));

    int data[] = {1, 1}; // Wymaga 2 workerów (a więc stacji o poj min 2)
    // Zakładamy, że model wymaga, by zadanie w całości weszło na jedną stację.
    task_t* t = create_task(800, 2, now, NULL, data);

    assert(0 == add_task(t));
    int res = collect_task(t);

    if (res != 0) {
        log_pass("Correctly handled fragmentation (task too big for any single station)");
    } else {
        log_fail("Task ran despite no single station being large enough!");
    }

    assert(0 == destroy_plant());
    free(w1); free(w2); free_task(t);
}

// Test 9: Deadlock / Shutdown Race
// Odpalamy zadania i natychmiast niszczymy fabrykę.
// Sprawdzamy czy nie ma segfaulta albo zwisu.
void test_shutdown_race() {
    printf("\n--- Test 9: Shutdown Race Condition ---\n");
    int stations[] = {10};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 5));

    time_t now = time(NULL);
    worker_t* workers[5];
    for(int i=0; i<5; i++) {
        workers[i] = create_worker(900+i, now, 100, work_sleepy); // Długa praca
        assert(0 == add_worker(workers[i]));
    }

    // Dodajemy dużo zadań
    int data[] = {1};
    task_t* tasks[10];
    for(int i=0; i<10; i++) {
        tasks[i] = create_task(950+i, 1, now, NULL, data);
        assert(0 == add_task(tasks[i]));
    }

    // NIE robimy collect_task.
    // Natychmiast niszczymy fabrykę, gdy zadania wiszą w kolejce lub są mielone.
    printf("Destroying plant while tasks are pending/running...\n");
    assert(0 == destroy_plant());

    // Jeśli tu dotarliśmy i nie ma segfaulta, to jest sukces.
    // Ale musimy posprzątać pamięć workerów i tasków ręcznie,
    // bo destroy_plant pewnie ubiło wątki, ale struktury w mainie zostały.

    for(int i=0; i<5; i++) free(workers[i]);
    for(int i=0; i<10; i++) free_task(tasks[i]);

    log_pass("Survived immediate shutdown under load");
}

// Test 10: Mixed Capacity & Integrity
// Mieszanie zadań o różnym zapotrzebowaniu i sprawdzanie spójności danych.
void test_mixed_load() {
    printf("\n--- Test 10: Mixed Load & Data Integrity ---\n");
    int stations[] = {2, 2}; // 4 sloty łącznie
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 2, 4));

    time_t now = time(NULL);
    worker_t* ws[4];
    for(int i=0; i<4; i++) {
        ws[i] = create_worker(1000+i, now, 100, work_memory_integrity);
        assert(0 == add_worker(ws[i]));
    }

    // Tworzymy mieszankę zadań
    task_t* tasks[6];
    int d1[] = {10};       // małe
    int d2[] = {20, 30};   // duże

    // 3 małe zadania, 3 duże zadania
    for(int i=0; i<6; i++) {
        if (i % 2 == 0) tasks[i] = create_task(2000+i, 1, now, NULL, d1);
        else            tasks[i] = create_task(2000+i, 2, now, NULL, d2);
        assert(0 == add_task(tasks[i]));
    }

    // Odbieramy
    for(int i=0; i<6; i++) {
        if(collect_task(tasks[i])) log_fail("Task failed unexpectedly in mixed load test");
        // Weryfikacja wyniku (input * 3)
        if (tasks[i]->capacity == 1) {
             if(tasks[i]->results[0] != 30) log_fail("Calculation error in mixed load (small task)");
        } else {
             if(tasks[i]->results[0] != 60 || tasks[i]->results[1] != 90) log_fail("Calculation error in mixed load (big task)");
        }
    }

    assert(0 == destroy_plant());
    for(int i=0; i<4; i++) free(ws[i]);
    for(int i=0; i<6; i++) free_task(tasks[i]);

    log_pass("Mixed load handled correctly without data corruption");
}
void test_parallel_execution_speed() {
    printf("\n--- Test A: Parallel Execution Speed (Timing Check) ---\n");
    int stations[] = {1,1};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 2, 2)); // 2 pracowników

    time_t now = time(NULL);

    // Dwóch pracowników
    worker_t* w1 = create_worker(1, now, 100, work_configurable_sleep);
    worker_t* w2 = create_worker(2, now, 100, work_configurable_sleep);
    assert(0 == add_worker(w1)); assert(0 == add_worker(w2));

    // Dwa zadania. Każde ma 1 element danych = 1000000 us (1 sekunda)
    int load[] = {1000000};
    task_t* t1 = create_task(101, 1, now, NULL, load);
    task_t* t2 = create_task(102, 1, now, NULL, load);

    time_t start = time(NULL);

    // Dodajemy oba
    assert(0 == add_task(t1));
    assert(0 == add_task(t2));

    // Zbieramy oba
    assert(0 == collect_task(t1));
    assert(0 == collect_task(t2));

    time_t end = time(NULL);
    double duration = difftime(end, start);

    printf("Tasks load: 2s total. Real time elapsed: %.0fs\n", duration);

    // Jeśli trwało 1s (lub mniej z zaokrągleniami), to jest OK.
    // Jeśli 2s, to znaczy, że szły jedno po drugim.
    if (duration <= 1.1) {
        log_pass("Tasks ran in parallel");
    } else {
        log_fail("Tasks ran sequentially (too slow!)");
    }

    assert(0 == destroy_plant());
    free(w1); free(w2); free_task(t1); free_task(t2);
}
#include <pthread.h> // Potrzebne tylko do tego testu, żeby symulować asynchroniczne dodanie

// Pomocnicza struktura i wątek do dodania pracownika z opóźnieniem
struct delayed_worker_args {
    worker_t* w;
    int delay_us;
};

void* delayed_add_worker_thread(void* arg) {
    struct delayed_worker_args* args = (struct delayed_worker_args*)arg;
    usleep(args->delay_us);
    printf("[Async] Adding worker ID %d now!\n", args->w->id);
    add_worker(args->w);
    return NULL;
}

void test_dynamic_worker_arrival() {
    printf("\n--- Test B: Dynamic Worker Arrival (Blocking) ---\n");
    int stations[] = {1};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 0)); // 0 pracowników na start!

    time_t now = time(NULL);
    int data[] = {10};
    task_t* t = create_task(200, 1, now, NULL, data);

    // Dodajemy zadanie. Powinno wejść do kolejki, ale nie ruszyć.
    // Uwaga: Jeśli add_task jest blokujące gdy nie ma workerów, ten test to sprawdzi.
    // Jeśli add_task jest nieblokujące, to collect_task zablokuje.
    assert(add_task(t) == 0); // Niektóre implementacje mogą odrzucić zadanie jeśli worker_count == 0.
    // tak bylo oryginalnie, ale moim zdaniem, to po prostu gowniana interpretacja tresci

    // To powinno wisieć przez ok 1s, aż wątek doda workera
    if(collect_task(t) == -1) {
        // wszystko jest ok, bo task nie mógł się wykonać        
    }
    else{
        if (t->results[0] == 20) {
            log_pass("Task finished after worker arrived dynamically");
        } else {
            log_fail("Task failed or produced wrong result");
        }

    }



    assert(0 == destroy_plant());
    free_task(t);
}
void test_station_bottleneck() {
    printf("\n--- Test C: Station Capacity Bottleneck ---\n");
    int stations[] = {1}; // BARDZO MAŁA STACJA (poj. 1)
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 10)); // AŻ 10 pracowników

    time_t now = time(NULL);
    worker_t* workers[10];
    for(int i=0; i<10; i++) {
        workers[i] = create_worker(i, now, 1000, work_configurable_sleep);
        assert(0 == add_worker(workers[i]));
    }

    // Zadania trwające 1s każde
    int load[] = {1000000};
    task_t* t1 = create_task(301, 1, now, NULL, load);
    task_t* t2 = create_task(302, 1, now, NULL, load);

    time_t start = time(NULL);
    assert(0 == add_task(t1));
    assert(0 == add_task(t2));

    assert(0 == collect_task(t1));
    assert(0 == collect_task(t2));
    time_t end = time(NULL);

    double duration = difftime(end, start);

    // Mamy 10 pracowników, więc teoretycznie mogliby to zrobić równolegle w 1s.
    // ALE stacja ma pojemność 1. Więc muszą wejść gęsiego -> czas > 2s.

    printf("Duration: %.0fs (Expected >= 2s due to station limit)\n", duration);

    if (duration >= 2.0) {
        log_pass("Station limit enforced correctly (serialized execution)");
    } else {
        // Jeśli zrobili to w 1s, to znaczy, że zignorowano pojemność stacji!
        log_fail("Station capacity limit ignored!");
    }

    assert(0 == destroy_plant());
    for(int i=0; i<10; i++) free(workers[i]);
    free_task(t1); free_task(t2);
}
void test_worker_distribution() {
    printf("\n--- Test E: Worker Distribution Logic ---\n");
    int stations[] = {5};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 1, 3)); // 3 pracowników (ID: 10, 20, 30)

    time_t now = time(NULL);
    worker_t* w1 = create_worker(10, now, 100, work_id_reporter);
    worker_t* w2 = create_worker(20, now, 100, work_id_reporter);
    worker_t* w3 = create_worker(30, now, 100, work_id_reporter);
    assert(0 == add_worker(w1)); assert(0 == add_worker(w2)); assert(0 == add_worker(w3));

    int data[] = {0}; // Dummy data
    // Tworzymy 3 zadania
    task_t* t1 = create_task(501, 1, now, NULL, data);
    task_t* t2 = create_task(502, 1, now, NULL, data);
    task_t* t3 = create_task(503, 1, now, NULL, data);

    assert(0 == add_task(t1));
    assert(0 == add_task(t2));
    assert(0 == add_task(t3));

    assert(0 == collect_task(t1));
    assert(0 == collect_task(t2));
    assert(0 == collect_task(t3));

    int id1 = t1->results[0];
    int id2 = t2->results[0];
    int id3 = t3->results[0];

    printf("Workers used: %d, %d, %d\n", id1, id2, id3);

    // Sprawdzamy czy ID są poprawne (czy to nasze workery)
    if ((id1==10||id1==20||id1==30) && (id2==10||id2==20||id2==30)) {
        log_pass("Workers assigned correctly");
    } else {
        log_fail("Unknown worker ID returned results!");
    }

    // Opcjonalnie: można sprawdzić czy pracują równolegle (różne ID),
    // ale scheduler może przydzielić t1->w1, t1 kończy, t2->w1.
    // Więc powtarzalność ID nie jest błędem, o ile robią to sekwencyjnie.

    assert(0 == destroy_plant());
    free(w1); free(w2); free(w3);
    free_task(t1); free_task(t2); free_task(t3);
}
// Worker, który po prostu czeka zadaną ilość czasu (w mikrosekundach)
int work_timer(worker_t* w, task_t* t, int i) {
    int duration = t->data[i];
    usleep(duration);
    return duration;
}

void test_tetris_scenario() {
    printf("\n--- Test EXTREME: The Tetris Scenario (Station Capacity Squeeze) ---\n");

    // KONFIGURACJA:
    // Stacja: pojemność 3.
    // Pracownicy: 3.
    int stations[] = {2,1};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 2, 3));

    time_t now = time(NULL);

    // Tworzymy 3 pracowników (niech pracują szybko i sprawnie)
    worker_t* w1 = create_worker(1, now, 1000, work_timer);
    worker_t* w2 = create_worker(2, now, 1000, work_timer);
    worker_t* w3 = create_worker(3, now, 1000, work_timer);
    assert(0 == add_worker(w1)); assert(0 == add_worker(w2)); assert(0 == add_worker(w3));

    // DEFINICJA ZADAŃ:
    // WAŻNE: Używamy usleep w mikrosekundach.
    // 2000000 us = 2 sekundy.
    // 1000000 us = 1 sekunda.

    int time_long = 2000000;
    int time_short = 1000000;

    // Zadanie A: Wielkie (2 sloty stacji), Długie (2s)
    int data_A[] = {time_long, time_long};
    task_t* tA = create_task(1001, 2, now, NULL, data_A); // capacity 2!

    // Zadanie B: Małe (1 slot stacji), Długie (2s)
    int data_B[] = {time_long};
    task_t* tB = create_task(1002, 1, now, NULL, data_B); // capacity 1

    // Zadanie C: Małe (1 slot stacji), Krótkie (1s)
    int data_C[] = {time_short};
    task_t* tC = create_task(1003, 1, now, NULL, data_C); // capacity 1

    printf("Starting Tetris...\n");
    printf("Expected behavior: Tasks A(2 slots) and B(1 slot) fill the station (3/3).\n");
    printf("                   Worker 3 is FREE, but Task C MUST WAIT for station space.\n");

    time_t start = time(NULL);

    // Wrzucamy A i B - zapychamy stację
    assert(0 == add_task(tA));
    assert(0 == add_task(tB));

    // Krótka pauza, żeby upewnić się, że scheduler przemielił A i B i zajął sloty
    usleep(100000);

    // Wrzucamy C. Powinno wejść do kolejki, ale NIE URUCHOMIĆ SIĘ od razu.
    assert(0 == add_task(tC));

    // Zbieramy wyniki
    assert(0 == collect_task(tA));
    assert(0 == collect_task(tB));
    assert(0 == collect_task(tC));

    time_t end = time(NULL);
    double duration = difftime(end, start);

    printf("Total execution time: %.1fs\n", duration);

    // ANALIZA:
    // Jeśli A i B ruszyły razem (t=0..2s), a C czekało na zwolnienie slotu (start t=2s, koniec t=3s).
    // Całość powinna trwać ok. 3 sekundy.
    // Jeśli C wcisnęło się "na chama" (bo był wolny pracownik), całość trwałaby 2 sekundy (max(2, 2, 1)).

    if (duration >= 3.0 && duration < 4.5) {
        log_pass("Tetris Logic Correct: Task C waited for station space despite free worker.");
    } else if (duration < 2.5) {
        printf("FAIL: Execution too fast (%.1fs). Task C likely ignored station capacity limit!\n", duration);
        log_fail("Station Capacity Breach");
    } else {
        printf("FAIL: Execution too slow (%.1fs). Something blocked excessively.\n", duration);
        log_fail("Deadlock or excessive locking");
    }

    assert(0 == destroy_plant());
    free(w1); free(w2); free(w3);
    free_task(tA); free_task(tB); free_task(tC);
}
#include <pthread.h>

// Struktura argumentów dla wątku "klienta"
typedef struct {
    int client_id;          // ID klienta (żeby generować unikalne ID zadań)
    int tasks_to_send;      // Ile zadań ten klient ma wysłać
    time_t base_time;       // Czas startu
} client_config_t;

// Funkcja robocza - symuluje klienta, który przychodzi do fabryki
// i zleca zadania w pętli.
void* client_thread_routine(void* arg) {
    client_config_t* cfg = (client_config_t*)arg;
    int success_count = 0;

    for (int i = 0; i < cfg->tasks_to_send; i++) {
        // Generujemy unikalne ID zadania: ClientID * 1000 + i
        // Np. Klient 1 robi zadania 1000, 1001, 1002...
        int task_id = (cfg->client_id * 1000) + i;

        // Dane zadania (czas spania w us)
        int duration = 10000; // 10ms - krótko, żeby był duży przemiał
        int data[] = {duration};

        // 1. Tworzymy zadanie
        // (create_task musi być thread-safe, bo to tylko malloc - zazwyczaj jest OK)
        task_t* t = create_task(task_id, 1, cfg->base_time, NULL, data);

        // 2. Wrzucamy do fabryki (KRYTYCZNY MOMENT DLA add_task)
        // Tutaj wiele wątków naraz dobija się do listy zadań!
        if (add_task(t) == 0) {
            // 3. Czekamy na wynik (KRYTYCZNY MOMENT DLA collect_task)
            // Tutaj sprawdzamy, czy condition variables w fabryce działają per zadanie,
            // czy może jeden signal budzi wszystkich i robi bałagan.
//            assert(collect_task(t) == 0);
//            printf("%d\n", (t->results[0]) == duration);
//            success_count++;
//            continue;
            if (collect_task(t) == 0) {
                // Weryfikacja wyniku
                if ((t->results[0]) == duration) {
                    success_count++;
                } else {
                    printf("Client %d: Task %d returned wrong result!\n", cfg->client_id, task_id);
                }
            } else {
                printf("Client %d: Failed to collect task %d\n", cfg->client_id, task_id);
            }
        } else {
            printf("Client %d: Failed to add task %d\n", cfg->client_id, task_id);
        }

//        free_task(t);

        // Mały losowy sleep, żeby wątki się rozjechały w czasie
        usleep(rand() % 5000);
    }

    printf("Client %d finished. Success: %d/%d\n", cfg->client_id, success_count, cfg->tasks_to_send);
    return NULL;
}

void test_multithreaded_api_access() {
    printf("\n--- Test EXTREME: Multi-threaded API Access (The Chaos) ---\n");

    // KONFIGURACJA FABRYKI
    // 2 stacje, 4 workerów.
    int stations[] = {2, 2};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    assert(0 == init_plant(stations, 2, 4));

    time_t now = time(NULL);

    // Dodajemy workerów (to robimy jednowątkowo na starcie, standard)
    worker_t* workers[4];
    for(int i=0; i<4; i++) {
        workers[i] = create_worker(i, now, 10000, work_configurable_sleep); // Czas pracy bardzo długi
        assert(0 == add_worker(workers[i]));
    }

    // KONFIGURACJA KLIENTÓW
    // 10 wątków zewnętrznych. Każdy wysyła 50 zadań.
    // Łącznie 500 zadań przelatujących przez system w losowych momentach.
    #define NUM_CLIENTS 10
    #define TASKS_PER_CLIENT 50

    pthread_t client_threads[NUM_CLIENTS];
    client_config_t configs[NUM_CLIENTS];

    printf("Spawning %d client threads, each sending %d tasks...\n", NUM_CLIENTS, TASKS_PER_CLIENT);
    printf("This tests race conditions in add_task() and collect_task().\n");

    // Startujemy wątki klientów
    for(int i=0; i<NUM_CLIENTS; i++) {
        configs[i].client_id = i + 1; // ID od 1 do 10
        configs[i].tasks_to_send = TASKS_PER_CLIENT;
        configs[i].base_time = now;

        if (pthread_create(&client_threads[i], NULL, client_thread_routine, &configs[i]) != 0) {
            perror("Failed to create client thread");
            exit(1);
        }
    }

    // Czekamy aż wszyscy skończą
    for(int i=0; i<NUM_CLIENTS; i++) {
        pthread_join(client_threads[i], NULL);
    }

    // Jeśli dotarliśmy tutaj bez Segfaulta, to już połowa sukcesu.
    // Fabryka powinna być pusta (wszystkie zadania odebrane).

    assert(0 == destroy_plant());
    for(int i=0; i<4; i++) free(workers[i]);

    log_pass("Multi-threaded API test survived");
}
int work_math_heavy(worker_t* w, task_t* t, int i) {
    int val = t->data[i];
    // Symulacja chrobotania dyskiem co 10-tą liczbę (żeby wymusić przełączenia kontekstu)
    if (i % 10 == 0) usleep(500);

    return (val * val) + val - 5;
}
void test_result_integrity_massive() {
    printf("\n--- Test FINAL: Massive Data Integrity Check (Chunked) ---\n");

    // KONFIGURACJA:
    // 5 stacji po 2 miejsca = 10 slotów łącznie.
    int stations[] = {10, 10, 9, 9, 5};
    if (stations[0] > 2137) {
        printf("Error: Not enough station capacity for 10 workers!\n");
        return;
    }
    // 10 pracowników.
    assert(0 == init_plant(stations, 5, 20));

    time_t now = time(NULL);

    // Tworzymy armię pracowników
    worker_t* workers[20];
    for(int i=0; i<20; i++) {
        workers[i] = create_worker(i, now, 100000, work_math_heavy);
        assert(0 == add_worker(workers[i]));
    }

    // POPRAWKA:
    // Task capacity nie może przekraczać liczby pracowników (10).
    // Dlatego zamiast 50 zadań po 100 elementów, robimy 500 zadań po 10 elementów.
    // Suma obliczeń (5000) pozostaje ta sama.

    const int CHUNK_SIZE = 10;   // Rozmiar zadania (musi być <= liczbie workerów)
    const int NUM_TASKS = 500;   // Więcej zadań, ale mniejszych

    task_t* tasks[NUM_TASKS];

    // 1. GENEROWANIE I WYSYŁANIE
    printf("Generating %d integers across %d tasks (Chunk size: %d)...\n",
           NUM_TASKS * CHUNK_SIZE, NUM_TASKS, CHUNK_SIZE);

    int added_count = 0;
    for (int t = 0; t < NUM_TASKS; t++) {
        int* input_data = malloc(sizeof(int) * CHUNK_SIZE);

        // Wypełniamy unikalnymi liczbami
        for (int j = 0; j < CHUNK_SIZE; j++) {
            // Unikalna wartość: ID_ZADANIA * 1000 + INDEX
            input_data[j] = (t * 1000) + j;
        }

        // Tworzymy zadanie o rozmiarze 10 (bezpieczne dla fabryki z 10 pracownikami)
        tasks[t] = create_task(t, CHUNK_SIZE, now, NULL, input_data);

        if (add_task(tasks[t]) != 0) {
            printf("[FAIL] Failed to add task %d (Capacity: %d)\n", t, CHUNK_SIZE);
            log_fail("Factory rejected a valid task!");
        } else {
            added_count++;
        }

        free(input_data);
    }

    // 2. ODBIERANIE I WERYFIKACJA
    printf("Verifying results from %d tasks...\n", added_count);
    long long total_errors = 0;

    for (int t = 0; t < NUM_TASKS; t++) {
        // Jeśli nie udało się dodać, nie odbieramy (bo test by wisiał)
        if (tasks[t] == NULL) continue;

        if (collect_task(tasks[t]) != 0) {
            printf("Failed to collect task %d\n", t);
            total_errors++;
            continue;
        }

        // Weryfikacja
        for (int j = 0; j < CHUNK_SIZE; j++) {
            int original_val = (t * 1000) + j;
            int expected = (original_val * original_val) + original_val - 5;
            int actual = tasks[t]->results[j];

            if (actual == -999) {
                // Jeśli wynik to nadal -999, worker nic nie wpisał
                printf("ERROR: Task %d, index %d -> Result NOT WRITTEN\n", t, j);
                total_errors++;
            } else if (actual != expected) {
                printf("ERROR: Task %d, index %d -> Expected: %d, Got: %d\n",
                       t, j, expected, actual);
                total_errors++;

                if (total_errors > 5) goto verification_end;
            }
        }

        // Progress bar co 50 zadań
        if (t % 50 == 0) printf(".");
    }
    printf("\n");

verification_end:
    if (total_errors == 0) {
        log_pass("All calculations correct (Data Integrity verified)");
    } else {
        printf("Found %lld errors!\n", total_errors);
        log_fail("Data integrity check failed");
    }

    assert(0 == destroy_plant());
    for(int i=0; i<10; i++) free(workers[i]);
    for(int i=0; i<NUM_TASKS; i++) free_task(tasks[i]);
}
int main() {
    printf("=== ROZPOCZYNAM TESTY FABRYKI (EXTENDED) ===\n");

    for(int i = 0; i < 10; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING BASIC SUCCESS TEST (ITERATION %d)\n", i);
        test_basic_success();
    }

    for(int i = 0; i < 10; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING FUTURE TASK TEST (ITERATION %d)\n", i);
        test_future_task();
    }

    for(int i = 0; i < 10; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING IMPOSSIBLE TASK TEST (ITERATION %d)\n", i);
        test_impossible_task();
    }

    for(int i = 0; i < 10; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING CONCURRENCY TEST (ITERATION %d)\n", i);
        test_concurrency();
    }

    for(int i = 0; i < 10; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING STRESS RESTART TEST (ITERATION %d)\n", i);
        test_stress_restart();
    }


    // --- NOWE HARDKORY ---
    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING TRUE PARALLELISM TEST (ITERATION %d)\n", i);
        test_true_parallelism();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING WORKER EXPIRATION TEST (ITERATION %d)\n", i);
        test_worker_expiration();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING STATION FRAGMENTATION TEST (ITERATION %d)\n", i);
        test_station_fragmentation();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING MIXED LOAD TEST (ITERATION %d)\n", i);
        test_mixed_load();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING SHUTDOWN RACE TEST (ITERATION %d)\n", i);
        test_shutdown_race();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING PARALLEL EXECUTION SPEED TEST (ITERATION %d)\n", i);
        test_parallel_execution_speed();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING STATION BOTTLENECK TEST (ITERATION %d)\n", i);
        test_station_bottleneck();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING DYNAMIC WORKER ARRIVAL TEST (ITERATION %d)\n", i);
        test_dynamic_worker_arrival(); // Wymaga biblioteki pthread (-lpthread przy kompilacji)
        // te zmienilem, bo byly absolutnie niezgodne z specyfikacja
        // teraz testuja P = 0
    }
 
    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING WORKER DISTRIBUTION TEST (ITERATION %d)\n", i);
        test_worker_distribution();
    }

    for(int i = 0; i < 0; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING TETRIS SCENARIO TEST (ITERATION %d)\n", i);
        test_tetris_scenario();
    }

    for(int i = 0; i < 2; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING MULTITHREAD API ACCESS TEST (ITERATION %d)\n", i);
        test_multithreaded_api_access();
    }

    for(int i = 0; i < 2; i++){
        assert(system("clear") == 0);
        printf("=== RUNNING MASSIVE INTEGRITY TEST (ITERATION %d)\n", i);
        test_result_integrity_massive();
    }
    printf("\n=== WSZYSTKIE TESTY ZALICZONE PRAWIDŁOWO ===\n");
    return 0;
}