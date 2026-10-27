CC      = g++
CFLAGS  = -Wall -Wextra -O2 -std=c++20

COMMON_OBJ = common.o
SERVER_OBJ = server.o
CLIENT_OBJ = client.o

.PHONY: all clean

all: kayles_server kayles_client

kayles_server: $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

kayles_client: $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_OBJ): server.cpp common.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(CLIENT_OBJ): client.cpp common.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(COMMON_OBJ): common.cpp common.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o kayles_server kayles_client