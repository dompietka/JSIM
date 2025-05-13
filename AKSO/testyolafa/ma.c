//
// Created by dominika on 11.04.25.
//

#include <stdio.h>

#include <stddef.h>

#include <stdlib.h>

#include <stdint.h>

#include <errno.h>

#include <stdbool.h>

#include "ma.h"

//struct na połaczenie - jeden struct to połączenie jednej pary bitów
typedef struct connection {
  moore_t * a_in; //automat, którego wejście jest podłączone
  moore_t * a_out; //automat, którego wyjście jest podłączone
  size_t in ; //numer bitu wejściowego
  size_t out; //numer bitu wyjściowego
  struct connection * next; //wskaźnik do następnego połączenia, potrzebny do listy
}
connection;

typedef struct moore {
  size_t n; //ile bitów wejscia
  size_t m; //ile bitów wyjscia
  size_t s; //ile bitów stanu

  transition_function_t t; //funckja zmian
  output_function_t y; //funkcja wyjsc

  uint64_t * state; //stan
  uint64_t * next_state; //pomocniczy wskaźnik na następny stan
  uint64_t * input; //wejście
  uint64_t * output; //wyjście

  connection ** connected; //tablica wskaźników na połączenia
  connection * outgoing_connections; //połączenia w ktorych wyjscie tego automatu jest podłączone
}
moore;

//funkcja pomocnicza, która oblicza rozmiar tablicy uint64_t 
//potrzebnej do przechowania n-elementowego ciagu bitów
size_t blocs(int n) {
  return (n + 63) / 64;
}

//funckja pomocnicza do alokowania pamieci, zwraca false w przypadku porażki
bool allocate_memory(moore_t * a) {
  a -> state = (uint64_t * ) malloc(sizeof(uint64_t) * blocs(a -> s));
  if (a -> state == NULL) {
    errno = ENOMEM;
    free(a);
    return false;
  }

  a -> next_state = (uint64_t * ) malloc(sizeof(uint64_t) * blocs(a -> s));
  if (a -> next_state == NULL) {
    errno = ENOMEM;
    free(a -> state);
    free(a);
    return false;
  }

  a -> input = (uint64_t * ) malloc(sizeof(uint64_t) * blocs(a -> n));
  if (a -> input == NULL) {
    errno = ENOMEM;
    free(a -> state);
    free(a -> next_state);
    free(a);
    return false;
  }

  a -> output = (uint64_t * ) malloc(sizeof(uint64_t) * blocs(a -> m));
  if (a -> output == NULL) {
    errno = ENOMEM;
    free(a -> state);
    free(a -> next_state);
    free(a -> input);
    free(a);
    return false;
  }

  a -> connected = (connection ** ) malloc(sizeof(connection * ) * a -> n);
  if (a -> connected == NULL) {
    errno = ENOMEM;
    free(a -> state);
    free(a -> next_state);
    free(a -> input);
    free(a -> output);
    free(a);
    return false;
  }

  return true;
}

//identycznościowa funckja wyjścia
static void identity(uint64_t * output, uint64_t
  const * state, size_t m, size_t) {
  for (size_t i = 0; i < blocs(m); i++) {
    output[i] = state[i];
  }
}

moore_t * ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, 
  output_function_t y, uint64_t const * q) {
  if (m == 0 || s == 0 || t == NULL || y == NULL || q == NULL) {
    errno = EINVAL;
    return NULL;
  }

  moore_t * a = (moore_t * ) malloc(sizeof(moore_t));
  if (a == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  a -> n = n;
  a -> m = m;
  a -> s = s;

  a -> t = t;
  a -> y = y;

  if (!allocate_memory(a))
    return NULL;

  a -> outgoing_connections = NULL;

  for (size_t i = 0; i < a -> n; i++) {
    a -> connected[i] = NULL;
  }

  for (size_t i = 0; i < blocs(s); i++) {
    a -> state[i] = q[i];
  }

  a -> y(a -> output, a -> state, m, s);

  return a;
}

moore_t * ma_create_simple(size_t n, size_t m, transition_function_t t) {
  if (n == 0 || m == 0 || t == NULL) {
    errno = EINVAL;
    return NULL;
  }

  moore_t * a = (moore_t * ) malloc(sizeof(moore_t));
  if (a == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  a -> n = n;
  a -> m = m;
  a -> s = m;

  a -> t = t;
  a -> y = identity;

  if (!allocate_memory(a))
    return NULL;

  a -> outgoing_connections = NULL;

  for (size_t i = 0; i < a -> n; i++) {
    a -> connected[i] = NULL;
  }

  for (size_t i = 0; i < blocs(a -> s); i++) {
    a -> state[i] = 0;
  }

  a -> y(a -> output, a -> state, m, m);

  return a;

}

void ma_delete(moore_t * a) {
  if (!a) return;

  if (ma_disconnect(a, 0, a -> n)) {
    int x = 1;
  } //odłącza połączenia od wejscia
  while (a -> outgoing_connections) { //odłącza połączenia wychodzące
    if (ma_disconnect(a -> outgoing_connections -> a_in,
        a -> outgoing_connections -> in, 1)) return;
  }

  free(a -> state);
  free(a -> next_state);
  free(a -> input);
  free(a -> output);
  free(a -> connected);
  free(a);
}

int ma_connect(moore_t * a_in, size_t in , moore_t * a_out,
  size_t out, size_t num) {
  if (!a_in || !a_out || !num || in + num - 1 >= a_in -> n ||
    out + num - 1 >= a_out -> m) {
    errno = EINVAL;
    return -1;
  }

  if (ma_disconnect(a_in, in, num)) {
    return -1;
  } //odłącza ewentualnie istniejące połączenia
  for (size_t i = 0; i < num; i++) {
    connection * c = (connection * ) malloc(sizeof(connection));
    if (!c) {
      errno = ENOMEM;
      return -1;
    }
    c -> a_in = a_in;
    c -> a_out = a_out;
    c -> in = in + i;
    c -> out = out + i;

    a_in -> connected[in + i] = c;

    //dodaje na poczatek listy połączeń automatu wyjściowego nowe polaczenie
    c -> next = a_out -> outgoing_connections;
    a_out -> outgoing_connections = c;

  }
  return 0;
}

//usuwa połączenie z listy i zwalnia pamięć
void delete_connection(connection * c) {
  connection * h = c -> a_out -> outgoing_connections;
  if (h == c) {
    c -> a_out -> outgoing_connections = h -> next;
    free(c);
    return;
  }
  while (h && h -> next != c) {
    h = h -> next;
  }

  h -> next = c -> next;
  free(c);
}

int ma_disconnect(moore_t * a_in, size_t in , size_t num) {
  if (!a_in || !num || in + num - 1 >= a_in -> n) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < num; i++) {
    connection * c = a_in -> connected[in + i];
    a_in -> connected[in + i] = NULL;
    if (c) delete_connection(c);
  }

  return 0;
}

int ma_set_input(moore_t * a, uint64_t
  const * input) {
  if (!a || !input || !a -> n) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < blocs(a -> n); i++) {
    a -> input[i] = input[i];
  }

  return 0;
}

int ma_set_state(moore_t * a, uint64_t
  const * state) {
  if (!a || !state) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < blocs(a -> s); i++) {
    a -> state[i] = state[i];
  }

  a -> y(a -> output, a -> state, a -> m, a -> s);

  return 0;
}

uint64_t
const * ma_get_output(moore_t
  const * a) {
  if (!a) {
    errno = EINVAL;
    return NULL;
  }
  return a -> output;
}

void get_input(moore_t * a) {
  if (!a) {
    errno = EINVAL;
    return;
  }

  for (size_t i = 0; i < a -> n; i++) {

    if (a -> connected[i]) {
      a -> input[(a -> connected[i] -> in) / 64] &=
        ~(1ULL << (a -> connected[i] -> in) % 64); //zeruje "i"ty bit inputu
      uint64_t bit = (a -> connected[i] -> a_out -> output[(a -> connected[i] -> out) / 64] >>
        (a -> connected[i] -> out) % 64);

      bit &= (1ULL); //bit który będzie podłączony
      a -> input[(a -> connected[i] -> in) / 64] |=
        (bit << (a -> connected[i] -> in) % 64); //ustawia "i"ty bit inputu
    }
  }
}

int ma_step(moore_t * at[], size_t num) {
  if (!at || !num) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < num; i++) {
    if (!at[i]) {
      errno = EINVAL;
      return -1;
    }
  }

  for (size_t i = 0; i < num; i++) { //wywołuje funkcje przejścia w kazdym automacie
    get_input(at[i]);
    at[i] -> t(at[i] -> next_state, at[i] -> input, at[i] -> state, at[i] -> n, at[i] -> s);
    for (size_t j = 0; j < blocs(at[i] -> s); j++)
      at[i] -> state[j] = at[i] -> next_state[j];
  }

  for (size_t i = 0; i < num; i++) { //aktualizuje sygnał wyjścia w kazdym automacie
    at[i] -> y(at[i] -> output, at[i] -> state, at[i] -> m, at[i] -> s);
  }

  return 0;
}

//funckje pomocnicze do debugowania
void print_automat(moore_t * a) {
  if (!a) return;

  //get_input(a);
  printf("Wejscie automatu:\n");
  for (size_t i = 0; i < blocs(a -> n); i++) {
    printf("%lu ", a -> input[i]);
  }
  printf("\n");

  printf("Stan automatu:\n");
  for (size_t i = 0; i < blocs(a -> s); i++) {
    printf("%lu ", a -> state[i]);
  }
  printf("\n");

  printf("Wyjscie automatu:\n");
  for (size_t i = 0; i < blocs(a -> m); i++) {
    printf("%lu ", a -> output[i]);
  }
  printf("\n");

  printf("Polaczenia automatu:\n");
  for (size_t i = 0; i < a -> n; i++) {
    if (a -> connected[i]) {
      printf("Wejscie %lu -> ", a -> connected[i] -> in);
      printf("Wyjscie %lu\n", a -> connected[i] -> out);
    }
  }

  printf("Polaczenia wyjsc:\n");
  connection * c = a -> outgoing_connections;
  while (c) {
    printf("Wejscie %lu -> ", c -> in);
    printf("Wyjscie %lu\n", c -> out);
    c = c -> next;
  }
}

void print_connection(connection * c) {
  if (!c) return;

  printf("Polaczenie:\n");
  printf("Wejscie %lu -> ", c -> in);
  printf("Wyjscie %lu\n", c -> out);

  if (c -> next) printf("Nastepne polaczenie:\n");
  else printf("Brak nastepnych polaczen\n");
}