AS=nasm
AS_FLAGS = -f elf64 -w+all -w+error -w-unknown-warning -w-reloc-rel
CC = gcc
# Need gnu11, gnu17 or gnu23 (pthread_barrier_t used in semaphore_test.c is a GNU extension).
CFLAGS = -std=gnu2x -pthread -Wall -Wextra -O2
LDFLAGS = -pthread -latomic

all: semaphore_test

semaphore_test: semaphore_test.o custom_sem.o
	$(CC) $(CFLAGS) -o $@ $^

semaphore_test.o: semaphore_test.c custom_sem.h
	$(CC) -c $(CFLAGS) -o $@ $<

custom_sem.o: custom_sem.asm
	$(AS) $(AS_FLAGS) -o $@ $<

clean:
	rm -f semaphore_test semaphore_test.o custom_sem.o

.PHONY: all clean
