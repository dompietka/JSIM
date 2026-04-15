#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <algorithm>
#include <map>
#include <cstring>
#include <csignal>
#include <queue>
#include <semaphore.h>
#include <stdlib.h>

using namespace std;

/* --- Definicje Stałych --- */
#define NAME_SIZE 5
#define STATE_SIZE 4
#define ACTION_SIZE 4


volatile sig_atomic_t shutdown_requested = 0;

// wyłapywanie sigint do ładnego zamknięcia
void handle_sigint(int sig) {
    (void)sig; // Ignorujemy numer sygnału (bo wiemy, że to SIGINT)
    shutdown_requested = 1;
}

// typy wiadomości w rurze zdarzeń
enum MsgType {
    MSG_NEW_TEST = 1,      
    MSG_ENV_STATE = 2,     
    MSG_POLICY_ACTION = 3,
    MSG_STDIN_EOF = 4     
};

// nagłówki wiadomości w rurze zdarzeń
typedef struct {
    int type;       
    int slot_id;    // ID slotu (indeks w tablicy active_tests), nie globalne ID testu! ???
    size_t len;    
} HeaderType;


//jedna "plansza"
typedef struct {
    int is_active;         
    long global_id;         
    char name[NAME_SIZE + 1];
    int assigned_policy_id;
    
    int env_pid;            
    int env_write_fd;       // Deskryptor do wysyłania akcji do wrappera
    
    char pending_state[STATE_SIZE]; // env -czeka->policy
    char pending_action[ACTION_SIZE]; // policy -czeka->env
} TestSlot;

// Stan procesu Polityki
typedef struct {
    int pid;                // PID procesu wrappera polityki
    int write_fd;           // Deskryptor do wysyłania stanu do polityki
    int is_busy;            // Czy polityka teraz pracuje? (1=Tak, 0=Nie)
    int to_pol;
    int from_pol;
    int kid_pid;
} PolicySlot;

sem_t pipe_mutex;

/* --- Funkcje Pomocnicze  --- */

// bezpieczny zapis i odczyt żeby nie przegapić EINTR i EPIPE, ale też obsłużyć częściowe zapisy/odczyty
bool safe_write(int fd, const void* buf, size_t count) {
    const char* ptr = (const char*)buf;
    size_t done = 0;
    sem_wait(&pipe_mutex);
    while (done < count) {
        ssize_t res = write(fd, ptr + done, count - done);
        if (res < 0) {
            if (errno == EINTR) continue; // Przerwane sygnałem, próbuj dalej
            if (errno == EPIPE) return false;   // Odbiorca umarł, ignorujemy
            perror("write failed");
            std::exit(1);
        }
        done += res;
    }
    sem_post(&pipe_mutex);
    return true;
}

// Bezpieczny odczyt (zwraca false jeśli EOF)
bool safe_read(int fd, void* buf, size_t count) {
    char* ptr = (char*)buf;
    size_t done = 0;
    sem_wait(&pipe_mutex);
    while (done < count) {
        ssize_t res = read(fd, ptr + done, count - done);
        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read failed");
            std::exit(1);
        }
        if (res == 0) return false;
        done += res;
    }
    sem_post(&pipe_mutex);
    return true;
}

// Bezpieczny odczyt z nagłówkiem

bool safe_read_w_hdr(int fd, void* hdr, void* buf, size_t count) {
    char* ptr = (char*)buf;
    HeaderType* hdr_ptr = (HeaderType*)hdr;
    size_t done = 0;
    sem_wait(&pipe_mutex);
    while (done < count) {
        ssize_t res = read(fd, (char*)hdr_ptr + done, count - done);
        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read failed");
            std::exit(1);
        }
        if (res == 0) return false;
        done += res;
    }
    count = hdr_ptr->len;
    done = 0;
    while (done < count) {
        ssize_t res = read(fd, ptr + done, count - done);
        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read failed");
            std::exit(1);
        }
        if (res == 0) return false;
        done += res;
    }
    sem_post(&pipe_mutex);
    return true;
}

bool safe_write_w_hdr(int fd, const void* hdr, const void* buf, size_t count) {
    const char* ptr = (const char*)buf;
    HeaderType* hdr_ptr = (HeaderType*)hdr;
    size_t done = 0;
    sem_wait(&pipe_mutex);
    while (done < count) {
        ssize_t res = write(fd, hdr_ptr + done, count - done);
        if (res < 0) {
            if (errno == EINTR) continue; // Przerwane sygnałem, próbuj dalej
            if (errno == EPIPE) return false;   // Odbiorca umarł, ignorujemy
            perror("write failed");
            std::exit(1);
        }
        done += res;
    }
    count = hdr_ptr->len;
    done = 0;
    while (done < count) {
        ssize_t res = write(fd, ptr + done, count - done);
        if (res < 0) {
            if (errno == EINTR) continue; // Przerwane sygnałem, próbuj dalej
            if (errno == EPIPE) return false;   // Odbiorca umarł, ignorujemy
            perror("write failed");
            std::exit(1);
        }
        done += res;
    }
    sem_post(&pipe_mutex);
    return true;
}

/* --- Procesy Pomocnicze --- */

// --- 1. Stdin Reader ---
// Czyta testy z wejścia i wysyła je do pipe'a
void run_stdin_reader(int out_pipe) {
    char buffer[NAME_SIZE];
    char newline_char;
    
    while (!shutdown_requested) {
        // Czytaj dokładnie NAME_SIZE znaków
        if (!safe_read(STDIN_FILENO, buffer, NAME_SIZE)) break;
        
        // Czytaj znak nowej linii
        if (!safe_read(STDIN_FILENO, &newline_char, 1)) break;

        // Wyślij do głównego procesu
        HeaderType hdr;
        hdr.type = MSG_NEW_TEST;
        hdr.slot_id = 0;
        hdr.len = NAME_SIZE;
        
        safe_write_w_hdr(out_pipe, &hdr, buffer, sizeof(hdr));
    }

    HeaderType hdr = {MSG_STDIN_EOF, 0, 0};
    safe_write(out_pipe, &hdr, sizeof(hdr));
    std::exit(0);
}

// --- 2. Environment Wrapper ---
// Uruchamia program środowiska, przekazuje stany do Main i odbiera akcje
void run_env(int slot_id, const char* name, const char* exe_path, 
                     char** extra_args, int event_pipe, int cmd_pipe) {
    
    // Pipe'y do komunikacji z rzeczywistym programem środowiska
    int to_env[2], from_env[2];
    if (pipe(to_env) < 0 || pipe(from_env) < 0) std::exit(1);

    pid_t pid = fork();
    if (pid == 0) {
        // --- Dziecko (Prawdziwe Środowisko) ---
        dup2(to_env[0], STDIN_FILENO);   // Czytaj z to_env
        dup2(from_env[1], STDOUT_FILENO);// Pisz do from_env
        
        // Sprzątanie deskryptorów
        close(to_env[0]); close(to_env[1]);
        close(from_env[0]); close(from_env[1]);
        close(event_pipe); close(cmd_pipe);

        // Przygotowanie argumentów execv
        // exe_path, name, extra_args..., NULL
        int count = 0;
        while(extra_args[count] != NULL) count++;
        
        char** args = (char**)malloc(sizeof(char*) * (count + 3));
        args[0] = (char*)exe_path;
        args[1] = (char*)name;
        for(int i=0; i<count; i++) args[i+2] = extra_args[i];
        args[count+2] = NULL;

        execv(exe_path, args);
        perror("execv env");
        std::exit(1);
    }

    // --- Wrapper ---
    close(to_env[0]);
    close(from_env[1]);

    char state_buf[STATE_SIZE];
    char action_buf[ACTION_SIZE + 1]; // +1 na \n
    char dummy_nl;

    while (true) {

        if (shutdown_requested) {
            close(to_env[1]);
            close(from_env[0]);
            kill(pid, SIGINT);
            waitpid(pid, NULL, 0);
            std::exit(0);
        }
        // 1. Czytaj STAN ze środowiska
        if (!safe_read(from_env[0], state_buf, STATE_SIZE)) break;
        if (!safe_read(from_env[0], &dummy_nl, 1)) break; // Połknij \n

        // 2. Wyślij STAN do Main
        HeaderType hdr = {MSG_ENV_STATE, slot_id, STATE_SIZE};
        safe_write_w_hdr(event_pipe, &hdr, state_buf, sizeof(hdr));

        // Jeśli to stan końcowy, kończymy
        if (state_buf[0] == 'T') break;

        // 3. Czekaj na AKCJĘ od Main
        if (!safe_read(cmd_pipe, action_buf, ACTION_SIZE)) break;
        action_buf[ACTION_SIZE] = '\n'; // Dodaj \n dla środowiska

        // 4. Wyślij AKCJĘ do środowiska
        safe_write(to_env[1], action_buf, ACTION_SIZE + 1);
    }

    // Czekaj na zakończenie dziecka
    waitpid(pid, NULL, 0);
    std::exit(0);
}

// --- 3. Policy Wrapper ---
// Trwała pętla: Odbiera (Stan) -> Pyta Politykę -> Odsyła (Akcję)
void run_policy(int policy_idx, PolicySlot* this_policy, const char* exe_path, 
                        char** extra_args, int event_pipe, int cmd_pipe) {

    int to_pol[2], from_pol[2];
    if (pipe(to_pol) < 0 || pipe(from_pol) < 0) std::exit(1);

    pid_t pid = fork();
    if (pid == 0) {
        // --- Dziecko (Prawdziwa Polityka) ---
        dup2(to_pol[0], STDIN_FILENO);
        dup2(from_pol[1], STDOUT_FILENO);
        
        close(to_pol[0]); close(to_pol[1]);
        close(from_pol[0]); close(from_pol[1]);
        close(event_pipe); close(cmd_pipe);

        int count = 0;
        while(extra_args[count] != NULL) count++;
        
        char** args = (char**)malloc(sizeof(char*) * (count + 3));
        args[0] = (char*)exe_path;
        // Konwersja int -> string dla ID polityki
        char idx_str[32];
        sprintf(idx_str, "%d", policy_idx);
        args[1] = idx_str;
        
        for(int i=0; i<count; i++) args[i+2] = extra_args[i];
        args[count+2] = NULL;

        execv(exe_path, args);
        perror("execv policy");
        std::exit(1);
    }

    // --- Wrapper ---
    close(to_pol[0]);
    close(from_pol[1]);

    this_policy->to_pol = to_pol[1];
    this_policy->from_pol = from_pol[1];
    this_policy->kid_pid = pid;

    // Protokół CmdPipe: [SlotID (int)] [State (STATE_SIZE)]
    struct {
        int slot_id;
        char state[STATE_SIZE];
    } cmd_pkt;

    char state_with_nl[STATE_SIZE + 1];
    char action_buf[ACTION_SIZE];
    char dummy_nl;

    while (true) {
        // 1. Czekaj na zlecenie od Main
        if (!safe_read(cmd_pipe, &cmd_pkt, sizeof(cmd_pkt))) break;


        // 2. Wyślij STAN do polityki
        std::memcpy(state_with_nl, cmd_pkt.state, STATE_SIZE);
        state_with_nl[STATE_SIZE] = '\n';
        safe_write(to_pol[1], state_with_nl, STATE_SIZE + 1);

        // 3. Czytaj AKCJĘ od polityki
        if (!safe_read(from_pol[0], action_buf, ACTION_SIZE)) break;
        if (!safe_read(from_pol[0], &dummy_nl, 1)) break;

        // 4. Odeślij AKCJĘ do Main
        // W nagłówku odsyłamy SlotID testu, żeby Main wiedział czyja to akcja
        HeaderType hdr = {MSG_POLICY_ACTION, cmd_pkt.slot_id, ACTION_SIZE};
        safe_write_w_hdr(event_pipe, &hdr, action_buf, sizeof(hdr));
    }
   
    exit(0);
}

/* --- MAIN --- */

int main(int argc, char* argv[]) {

    sem_init(&pipe_mutex, 0, 1);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);

    if (argc < 6) { perror("za malo arg"); return 1; }
    const char* policy_path = argv[1];
    const char* env_path = argv[2];
    int max_concurrent_policy_calls = atoi(argv[3]);
    int max_concurrent_calls = atoi(argv[4]);
    int max_active_envs = atoi(argv[5]);

    char** extra_args = &argv[6]; // Dodatkowe argumenty

    // Główna rura zdarzeń (Wszyscy -> Main)  //main ma otwarte oba konce zeby do forkowania ok
    int event_pipe[2];
    if (pipe(event_pipe) < 0) {perror("pipe error"); return 1;}
    fcntl(event_pipe[0], F_SETFD, FD_CLOEXEC);

    pid_t stdin_pid = fork();
    if (stdin_pid == 0) {   //dziecko
        close(event_pipe[0]);  //zamykam czytanie eventow
        run_stdin_reader(event_pipe[1]); // exit w środku, wait na koncu
    }
    //struktury


    TestSlot slots[max_active_envs];
    PolicySlot policies[max_concurrent_policy_calls];
    int policies_count = 0;

    // Kolejki 
    queue<int> ready_for_policy_q; // Sloty czekające na policy
    queue<int> ready_for_env_q;  // Sloty czekające na env (mają akcję)
    queue <pair<int, string>> pending_tests_q;   // Testy czekające na start
    queue <int> free_slots_q; //wolne sloty na test
    queue <int> free_policies_q; //wolne polityki

    //mapy
    map<int, string> test_nr_to_name;
    map<int, string> test_res;

    // Liczniki
    int active_envs_count = 0;      // Ile slotów zajętych
    int active_env_calls = 0;    // Env working + Policy working
    int active_policy_calls = 0;    // Policy working
    
    long global_test_counter = 0;   // Nadawanie ID
    long next_output_test = 0;
    bool stdin_closed = false;

    // Buffer odczytu
    string data_buf;

    /* --- GŁÓWNA PĘTLA --- */
    while (!shutdown_requested) {
        //zakończenie jeśli wszystko przetworzone, wypisywanie się zakończy samo
        if (stdin_closed && active_envs_count == 0 && pending_tests_q.empty()) {
            break;
        }

        //odczyt zdarzenia
        HeaderType hdr;
        if (!safe_read_w_hdr(event_pipe[0], &hdr, &data_buf, sizeof(hdr))) break;
        
        // obsluga zdarzenia
        if (hdr.type == MSG_NEW_TEST) {
            data_buf[hdr.len] = '\0'; 
            pair <int, string> temp;
            temp.first=global_test_counter;
            temp.second=data_buf;

            pending_tests_q.push(temp);
            global_test_counter++;

        }
        else if (hdr.type == MSG_ENV_STATE) {
            int slot = hdr.slot_id;
            active_env_calls--;

            if (hdr.len > 0 && data_buf[0] == 'T') {
                // --- TEST ZAKOŃCZONY ---
                int nr =slots[slot].global_id;
                test_res[nr] = string(slots[slot].name) + " " + data_buf.substr(1, STATE_SIZE - 1) + "\n";
                while (next_output_test == nr && test_res.find(nr)!=test_res.end()){
                    safe_write(STDOUT_FILENO, test_res[nr].c_str(), test_res[nr].size());
                    next_output_test++;
                    nr++;
                }

                close(slots[slot].env_write_fd); //zamykam pipe
                waitpid(slots[slot].env_pid, NULL, 0); //ok bo on zrobi exit zaraz
                
                // Zwolnij slot
                slots[slot].is_active = 0;
                free_slots_q.push(slot);
                active_envs_count--;
            } 
            else {
                // --- STAN POŚREDNI -> DO POLITYKI ---
                memcpy(slots[slot].pending_state, data_buf.c_str(), STATE_SIZE);
                ready_for_policy_q.push(slot);
            }
        }
        else if (hdr.type == MSG_POLICY_ACTION) {
            int slot = hdr.slot_id;
            int pol_id = slots[slot].assigned_policy_id;
        
            // Zwolnij politykę
            policies[pol_id].is_busy = 0;
            free_policies_q.push(pol_id);
            active_policy_calls--;

            slots[slot].assigned_policy_id = -1;

            memcpy(slots[slot].pending_action, data_buf.c_str(), ACTION_SIZE);
            ready_for_env_q.push(slot);
        }
        else if (hdr.type == MSG_STDIN_EOF) {
            stdin_closed = true;
            waitpid(stdin_pid, NULL, 0);
        }
                    cerr << "hello2";
        // 3. SZEREGOWANIE (SCHEDULER)
        // Musi być wykonywane w pętli dopóki da się coś zrobić
        bool progress_possible = true;
        while (progress_possible) {

            progress_possible = false;

            // A. Uruchom polityke jesli sie da
            if(!ready_for_policy_q.empty() && 
                   active_policy_calls < max_concurrent_policy_calls && 
                   active_policy_calls + active_env_calls < max_concurrent_calls) {
                
                // Znajdź lub stwórz politykę
                int pol_id = -1;
                if (!free_policies_q.empty()) {
                    pol_id = free_policies_q.front();
                    free_policies_q.pop();
                } else {
                    //jeśli nie ma wolnej, ale możemy stworzyć to lets go
                    if (policies_count < max_concurrent_policy_calls) {
                        policies_count++;
                        pol_id = policies_count;
                        
                        // Spawn process
                        int policy_fd[2];
                        if (pipe(policy_fd) < 0) exit(1);
                        
                        pid_t pid = fork();
                        if (pid == 0) { //nowy proces
                            close(event_pipe[0]); //nie potrzebuje czytać eventow
                            close(policy_fd[1]);  //nie potrzebuje pisac do siebie
                            run_policy(pol_id, &policies[pol_id], policy_path, extra_args, event_pipe[1], policy_fd[0]);
                                                                                //pisze na eventach czytam z policy managera
                        }
                        close(policy_fd[0]); //nie potrzebuje czytac z dziecka
                        fcntl(policy_fd[1], F_SETFD, FD_CLOEXEC);
                        
                        policies[pol_id].pid = pid;
                        policies[pol_id].write_fd = policy_fd[1];
                        policies[pol_id].is_busy = 0;
                    } 
                    
                }

                // Pobierz test
                int slot = ready_for_policy_q.front();
                ready_for_policy_q.pop();

                // Wyślij zlecenie do polityki
                struct { int s; char d[STATE_SIZE]; } pkt;
                pkt.s = slot;
                memcpy(pkt.d, slots[slot].pending_state, STATE_SIZE);
                safe_write(policies[pol_id].write_fd, &pkt, sizeof(pkt)); //ona to odczyta od razu bo jest wolna

                // Aktualizuj stan
                policies[pol_id].is_busy = 1;
                slots[slot].assigned_policy_id = pol_id;
                active_policy_calls++;

                if (active_policy_calls < max_concurrent_policy_calls) progress_possible=true;
            }

            // B. Wyślij Akcje do Środowisk (jeśli są zadania i CPU)
            if (!ready_for_env_q.empty() && active_env_calls+active_policy_calls < max_concurrent_calls) {
                int slot = ready_for_env_q.front();
                ready_for_env_q.pop();
                
                safe_write(slots[slot].env_write_fd, slots[slot].pending_action, ACTION_SIZE);
                active_env_calls++;

                if(active_env_calls+active_policy_calls < max_concurrent_calls) progress_possible=true;
            }

            // C. Uruchom Nowe Testy (jeśli są nazwy i wolne sloty RAM)
            // Uwaga: Start środowiska też liczy się jako computation (do momentu pierwszego stanu)
            if (!pending_tests_q.empty() && 
                   active_envs_count < max_active_envs && 
                   active_env_calls+active_policy_calls < max_concurrent_calls) {
                
                if (free_slots_q.empty()) break; // Powinno być zbędne przy active_envs check, ale dla pewności
                int slot = free_slots_q.front();
                free_slots_q.pop();
                
                auto name = pending_tests_q.front();
                pending_tests_q.pop();

                // Inicjalizacja slotu
                slots[slot].is_active = 1;
                memcpy(slots[slot].name, name.second.c_str(), NAME_SIZE + 1);
                slots[slot].global_id=name.first;
                slots[slot].assigned_policy_id = -1;

                // Start Wrappera Env
                int env_fd[2];
                if (pipe(env_fd) < 0) exit(1);

                pid_t pid = fork();
                if (pid == 0) { //dziecko
                    close(event_pipe[0]); //nie musze czytac eventow
                    close(env_fd[1]); //nie musze pisac do siebie
                    run_env(slot, slots[slot].name, env_path, extra_args, event_pipe[1], env_fd[0]);
                }
                close(env_fd[0]); //nie musze czytac od dziecka
                fcntl(env_fd[1], F_SETFD, FD_CLOEXEC);

                slots[slot].env_pid = pid;
                slots[slot].env_write_fd = env_fd[1]; //slot wie ktoredy pisac do srodowiska

                active_envs_count++;
                active_env_calls++; // Env pracuje nad stanem początkowym

                if(active_envs_count < max_active_envs && 
                   active_env_calls+active_policy_calls < max_concurrent_calls) progress_possible=true;
            }
            if (pending_tests_q.empty() && ready_for_env_q.empty() && ready_for_policy_q.empty()) progress_possible=false;
        }

    }
    /* --- SPRZĄTANIE --- */

    if(shutdown_requested){
        while (next_output_test <= global_test_counter){
            if (test_res.find(next_output_test)!=test_res.end())
                safe_write(STDOUT_FILENO, test_res[next_output_test].c_str(), test_res[next_output_test].size());
            else{
                string res = test_res[next_output_test] + " ERROR: TEST NOT FINISHED DUE TO SHUTDOWN";
                safe_write(STDOUT_FILENO, res.c_str(), test_res[next_output_test].size());
            }
            next_output_test++;
        }

        // Zabij Stdin Reader
        if (!stdin_closed) {
            kill(stdin_pid, SIGKILL);
        }
        waitpid(stdin_pid, NULL, 0);

        // Zabij Polityki
        for (int i = 0; i < policies_count; i++) {
            close(policies[i].write_fd);
            close(policies[i].to_pol);
            close(policies[i].from_pol);
            kill(policies[i].kid_pid, SIGINT);
            waitpid(policies[i].kid_pid, NULL, 0);
            kill(policies[i].pid, SIGKILL);
            waitpid(policies[i].pid, NULL, 0);
        }

        // Zabij aktywne środowiska
        for (int i = 0; i < max_active_envs; i++) {
            if (slots[i].is_active) {
                close(slots[i].env_write_fd);
                //nie musze zabijac bo sam wie i zabije dziecko
                waitpid(slots[i].env_pid, NULL, 0);
            }
        }

        close(event_pipe[0]);
    } else {
        // Zabij Stdin Reader
        if (!stdin_closed) {
            kill(stdin_pid, SIGKILL);
        }
        waitpid(stdin_pid, NULL, 0);

        // Zabij Polityki
        for (int i = 0; i < policies_count; i++) {
            close(policies[i].write_fd);
            close(policies[i].to_pol);
            close(policies[i].from_pol);
            kill(policies[i].kid_pid, SIGTERM);
            waitpid(policies[i].kid_pid, NULL, 0);
            kill(policies[i].pid, SIGKILL);
            waitpid(policies[i].pid, NULL, 0);
        }

        
        close(event_pipe[0]);
    }

    return shutdown_requested ? 2 : 0;
}