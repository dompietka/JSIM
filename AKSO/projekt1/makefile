# Deklaracja pseudocelów
.PHONY: all libma.so clean tests

#zmienne
CC = gcc
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O0 -g
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup
LIBRARY_NAME = libma.so
# Pliki źródłowe dla biblioteki
LIB_SRC = ma.c memory_tests.c 
LIB_OBJ = $(LIB_SRC:.c=.o)

all: libma.so

# Kompilacja biblioteki
libma.so: $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: ma_example.c libma.so
	$(CC) $(CFLAGS) -o test ma_example.c ./libma.so

# Usuwanie plików tymczasowych i wynikowych
clean:
	rm -f *.o libma.so test
