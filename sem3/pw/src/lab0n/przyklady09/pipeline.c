#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "err.h"
#include "pipeline-utils.h"


int main(int argc, char* argv[])
{
    // Read descriptor of previous pipe.
    int prev_read_dsc = -1;

    fprintf(stderr, "Pid %d starts %s\n", getpid(), argv[0]);
    print_open_descriptors();

    // Spawn children calling argv[1], ..., argv[argc - 1].
    for (int i = 1; i < argc; ++i) {
        int pipe_dscs[2];
        if (i < argc - 1) {
            ASSERT_SYS_OK(pipe(pipe_dscs)); //tworzenie łącza 
            fprintf(stderr, "Created pipe [%d (read), %d (write)] for process %d -> %d\n",
                    pipe_dscs[0], pipe_dscs[1], i, i + 1);
        }

        sleep(1);  //czytelnosc logow

        pid_t pid = fork();
        ASSERT_SYS_OK(pid);

        if (!pid) { //jeśli jestem dzieckiem
            // For all programs but the first, replace stdin with the previous pipe's reading end.
            if (i > 1) {
                fprintf(stderr, "Child %d: replacing stdin descriptor %d with a copy of %d\n", i, STDIN_FILENO, prev_read_dsc);
                ASSERT_SYS_OK(close(pipe_dscs[1])); //zamykam pisanie
                ASSERT_SYS_OK(dup2(prev_read_dsc, STDIN_FILENO)); // podmieniam czytanie
                ASSERT_SYS_OK(close(prev_read_dsc)); //zamykam stare czytanie
                prev_read_dsc = -1;
            }

            sleep(1);  //czytelnosc logow
            // For all programs but the last, replace stdout with the current pipe's writing end.
            if (i < argc - 1) {
                ASSERT_SYS_OK(close(pipe_dscs[0])); //zamykam czytanie
                 fprintf(stderr, "Child %d (pid %d): dup2(pipefd[1]=%d -> STDOUT=%d)\n",
                        i, getpid(), pipe_dscs[1], STDOUT_FILENO);
                ASSERT_SYS_OK(dup2(pipe_dscs[1], STDOUT_FILENO)); //podmieniam pisanie
                ASSERT_SYS_OK(close(pipe_dscs[1])); //zamykam stare pisanie
            }

                int _placeholder = 1024;

                fprintf(stderr, "Child %d: replacing stdout descriptor %d with a copy of %d\n", i, STDOUT_FILENO, _placeholder);
                // FIXME:
            }

            fprintf(stderr, "Pid %d execs %s\n", getpid(), argv[i]);
            print_open_descriptors();
            ASSERT_SYS_OK(execlp(argv[i], argv[i], NULL));
        }

        sleep(1);

        // Close the read descriptor we gave to our child.
        if (i > 1) {
            // FIXME:
        }
        // Close writing end of pipe, keep the reading end.
        if (i < argc - 1) {
            // FIXME:
        }
    }

    for (int i = 1; i < argc; ++i) {
        ASSERT_SYS_OK(wait(NULL));
    }

    fprintf(stderr, "Pid %d finished %s\n", getpid(), argv[0]);
    print_open_descriptors();

    return 0;
}
