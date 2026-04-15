
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

void write_trace(const char* type) {
    char* path = getenv("TRACE_FILE");
    if (!path) return;
    
    int fd = -1;
    // Retry loop: Under high load (Stress Test), open might fail with EMFILE or contention.
    // We try 10 times with small sleeps to ensure we don't drop events.
    for (int i = 0; i < 10; i++) {
        // O_APPEND ensures atomic writes on POSIX for small buffers
        fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd != -1) break;
        usleep(1000); // 1ms wait
    }
    
    if (fd == -1) return; // If it still fails, we lose the event (likely causing violation)
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long long now = (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
    
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%lld %s %d\n", now, type, getpid());
    if (len > 0) write(fd, buf, len);
    close(fd);
}

#include <signal.h>

#ifndef ACTION_SIZE
#define ACTION_SIZE 128
#endif
#ifndef STATE_SIZE
#define STATE_SIZE 128
#endif

// Global flags to track active states
volatile sig_atomic_t pol_active = 0;
volatile sig_atomic_t cpu_active = 0;

void handle_sig(int sig) { 
    (void)sig; 
    // Close inner resource (CPU) first if active
    if (cpu_active) write_trace("CPU_BUSY_END");
    // Close outer resource (Policy) if active
    if (pol_active) write_trace("POL_BUSY_END");
    exit(0); 
}

void print_padded(const char* msg, int size) {
    int len = strlen(msg);
    if (len > size) len = size;
    fwrite(msg, 1, len, stdout);
    for (int i = len; i < size; i++) fputc('z', stdout);
    fputc('\n', stdout);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sig);
    int verbose = (getenv("VERBOSE") != NULL);
    if (argc < 4) return 1;
    int sleep_ms = atoi(argv[3]);
    
    char buf[STATE_SIZE + 1];
    char newline_check;

    while (1) {
        int r = fread(buf, 1, STATE_SIZE, stdin);
        if (r < STATE_SIZE) break;
        buf[STATE_SIZE] = 0;
        fread(&newline_check, 1, 1, stdin);

        if (verbose) fprintf(stderr, "[MOCK_POL %d] Received State: %.10s...\n", getpid(), buf);

        pol_active = 1;
        write_trace("POL_BUSY_START");
        
        cpu_active = 1;
        write_trace("CPU_BUSY_START");
        if(sleep_ms > 0) usleep(sleep_ms * 1000);
        
        // FIX: Log ENDs before sending action
        write_trace("CPU_BUSY_END");
        cpu_active = 0;

        write_trace("POL_BUSY_END");
        pol_active = 0;
        
        print_padded("Action", ACTION_SIZE);
    }
    return 0;
}
