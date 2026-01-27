PROGRAMS = inc_thread_test_naive
AS=nasm
AS_FLAGS = -f elf64 -w+all -w+error -w-unknown-warning -w-reloc-rel
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -O2
LDFLAGS = -pthread -latomic

all: $(PROGRAMS)

err.o: err.c err.h

inc_thread_naive.o: inc_thread_naive.asm
	$(AS) $(AS_FLAGS) -o $@ $<

inc_thread_test.o: inc_thread_test.c inc_thread.h err.h
	$(CC) -c $(CFLAGS) -o $@ $<

inc_thread_test_naive: inc_thread_test.o inc_thread_naive.o err.o
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: all clean

clean:
	rm -rf $(PROGRAMS) *.o
