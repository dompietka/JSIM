#ifdef NDEBUG
#undef NDEBUG
#endif

#include "ma.h"
#include "memory_tests.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/** MAKRA SKRACAJĄCE IMPLEMENTACJĘ TESTÓW **/

// To są możliwe wyniki testu.
#define PASS 0
#define FAIL 1
#define WRONG_TEST 2

// Oblicza liczbę elementów tablicy x.
#define SIZE(x) (sizeof x / sizeof x[0])

#ifdef NDEBUG
#undef NDEBUG
#endif

#include "ma.h"
#include "memory_tests.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PASS 0
#define FAIL 1
#define WRONG_TEST 2

#define SIZE(x) (sizeof x / sizeof x[0])

// Zmodyfikowane makro assert – wypisuje numer linii przed sprawdzeniem warunku.
#undef assert
#define assert(f) \
  do { \
    printf("assert at line %d: %s\n", __LINE__, #f); \
    if (!(f)) { \
      return FAIL; \
    } \
  } while (0)

// Reszta kodu pozostaje niezmieniona...


#define V(code, where) (((unsigned long)code) << (3 * where))

/** WŁAŚCIWE TESTY **/
void t_add_timestamps(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
    next_state[0] = input[0]+input[1]; // sumujemy 2 liczby z wejścia
    next_state[1] = old_state[1] + 1; //zwiększamy licznik o 1
  }

  void t_shift3(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    next_state[0] = input[0] >> 3; //dzielimy przez 8
  }

  void t_3nplus1(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    next_state[0] = 3*input[0] + 1;
  }

  void t_plus1_mod8(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    next_state[0] = (input[0] + 1)%8;
  }


  const uint64_t M = 2137;
  void y_mulM(uint64_t *output, uint64_t const *state, size_t, size_t) {
    output[0] = (state[0] * state[1])%M;
  }

  //automat 0 dokonuje obliczeń na podsatwie wyjść automatów 0 i 1
  //zaś one dokonują obliczeń na podstawie automatu 0
  //
  //do tego na boku funkcjonuje automat disruptor, który łączy się na różne dziwne sposoby, jednak nie wpływa on na wynik
  //(podłącza się on jedynie do pierwszych 3 bitów automatu 1, które są nieistotne bo automat ten wykonuje dzielenie przez 8)
  //
  //UWAGA: zachęcam do spróbowania uruchomienia tego testu bez disruptora
  static int disruptor(void) {
    uint64_t zero = 2;
    const uint64_t q[2] = {17, 1};
    const uint64_t *y[3];
    moore_t *a[3];
    moore_t *disruptor = NULL;

    //automat 0 z każdym obrotem dodaje 2 liczby, które dostaje na wejściu oraz mnoży je przez licznik, który zwiększa co obrót
    //automat 1 dzieli wejście przez 8
    //automat 2 mnoży wejście 3 i dodaje 1
    a[0] = ma_create_full(100, 20, 80, t_add_timestamps, y_mulM, q);
    a[1] = ma_create_simple(20, 20, t_shift3);
    a[2] = ma_create_simple(20, 20, t_3nplus1);
    assert(a[0]);
    assert(a[1]);
    assert(a[2]);

    /*
      Początkowe stany automatów:
      ?, ? -> 17, 1 -> 17
      ? -> 0 -> 0
      ? -> 0 -> 0
    */


    y[0] = ma_get_output(a[0]);
    y[1] = ma_get_output(a[1]);
    y[2] = ma_get_output(a[2]);
    assert(y[0] != NULL);
    assert(y[0][0] == 17);
    assert(y[1] != NULL);
    assert(y[1][0] == 0);
    assert(y[2] != NULL);
    assert(y[2][0] == 0);


    ma_connect(a[1], 0, a[0], 0, 20);
    ma_connect(a[2], 0, a[0], 0, 20);
    ma_step(&(a[1]), 2);
    ma_connect(a[0], 0, a[1], 0, 20);
    ma_connect(a[0], 64, a[2], 0, 20);
    /*
      Obecne stany automatów
      2, 52 -> 17, 1 -> 17
      17 -> 2 -> 2
      17 -> 52 -> 52
    */
    assert(y[0][0] == 17);
    assert(y[1][0] == 2);
    assert(y[2][0] == 52);

    uint64_t out0 = 17, out1 = 2, out2 = 52;
    for (uint64_t i = 2; i < 200; i++) {
      //printf("Loop for i = %ld \n", i);

      //Robimy krok na wszystkich automatach
      assert(ma_step(a, 3) == 0);

      //Ręcznie liczymy nowe wyjścia automatów
      uint64_t out0_copy = out0;
      out0 = ((out1 + out2) * i)%M;
      out1 = out0_copy/8;
      out2 = 3*out0_copy + 1;

      //Sprawdzamy
      assert(y[0][0] == out0);
      assert(y[1][0] == out1);
      assert(y[2][0] == out2);

      //Disruptor sieje ferment (bez wpływu na pozostałe wyniki)
      if (!disruptor) {
        //if (i%3 == 0) printf("i = %ld - TRIGGERS disruptor CREATION\n", i);
        if (i %3 == 0) disruptor = ma_create_simple(3, 3, t_plus1_mod8);
      } else {
        if (i%4 == 0) {
          //printf("i = %ld - TRIGGERS DELETE\n", i);
          ma_delete(disruptor);
          disruptor = NULL;
        } else {
          if (i%7 == 2 || i%7 == 3) {
            //podłączamy disruptor do 3 pierwszych bitów wejścia
            //printf("i = %ld - TRIGGERS CONNECT\n", i);
            assert(ma_connect(a[1], 0, disruptor, 0, 3) == 0);
          } else {
            if (i%11 < 5) {
              //podłączamy wejście disruptora do fragmentu losowego wyjścia a0, a1, a2
              //printf("i = %ld - TRIGGERS WEIRD LINK UP\n", i);
              size_t rand_mod3 = (i%5+ 2)%3;
              size_t another = (rand_mod3 + 2 + i%8)%3;
              assert(ma_connect(disruptor, 0, a[rand_mod3], 5, another + 1) == 0);
            } else {
              //printf("i = %ld - TRIGGERS DISCONNECTS\n", i);
              assert(ma_disconnect(disruptor, 0, 3) == 0);
              assert(ma_disconnect(a[1], 0, 3) == 0);
              assert(ma_set_input(disruptor, &zero) == 0);
            }
          }
        }
      }
    }

    ma_delete(a[0]);
    ma_delete(a[1]);
    ma_delete(a[2]);
    if (disruptor) {
      ma_delete(disruptor);
    }
    printf("PASS\n");
    return PASS;
  }
void t_times3(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = 3 * input[0];
}

void t_plus(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

void t_shift(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
  next_state[0] = input[0] >> 60;
  if (s > 64 && n > 64) next_state[1] = input[1];
}

void y_plus1(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] + 1;
}

void y_times10(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] *10;
}

// Dla s = 1, 96, funkcja ustawia nowy stan jako negację wejścia.
void transition_test2_f(uint64_t *next_state, uint64_t const *input,
	uint64_t const *state, size_t n, size_t s) {

    (void) n;
    (void) state;

	if (s == 1)
		next_state[0] = !input[0];
	else if (s == 96) {
		next_state[0] = ~input[0];
		next_state[1] = ~input[1] & 0x00000000ffffffff;
	}
}

void transition_add_f(uint64_t *next_state, uint64_t const *input,
	uint64_t const *state, size_t n, size_t s) {

	next_state[1] = state[1] + input[1];
}


static int test2(void) {
	
	moore_t *a1 = ma_create_simple(100, 100, &transition_add_f);
	ma_set_state(a1, (uint64_t [2]) {0, 1});
	uint64_t const *a1_out = ma_get_output(a1);
	ma_set_input(a1, (uint64_t [2]) {0, 0});

	// Argumenty poza zakresem, więc połączenie nie powinno się wykonać.
	ma_connect(a1, 64, a1, 64, 100);
	ma_step(&a1, 1);
	assert(a1_out[1] == (uint64_t) 1);

	// Teraz już powinno się wykonać.
	ma_connect(a1, 64, a1, 64, 36);

	for (size_t i = 0; i < 5; i++)
		ma_step(&a1, 1);
	assert(a1_out[1] == 32);

	const size_t a2_size = 8;
	moore_t *a2[a2_size];
	uint64_t const *a2_output[a2_size];

	for (size_t i = 0; i < a2_size; i++) {
		a2[i] = ma_create_simple(1, 1, &transition_test2_f);
		a2_output[i] = ma_get_output(a2[i]);
		assert(a2_output[i][0] == 0);
		
		ma_set_input(a2[i], (uint64_t [1]) {1});
		ma_connect(a2[i], 0, a1, 0, 1);
		ma_step(&a2[i], 1);
		assert(a2_output[i][0] == 1);
	}

	moore_t *a3 = ma_create_simple(96, 96, &transition_test2_f);
	ma_set_state(a3, (uint64_t [2]) {~(uint64_t) 0, 0x00000000ffffffff});
	uint64_t const *a3_output = ma_get_output(a3);
	assert(a3_output[1] == 0x00000000ffffffff && a3_output[0] == ~(uint64_t) 0);
	
	for (size_t i = 0; i < 96; i++)
		ma_connect(a3, i, a2[i % 8], 0, 1);
	ma_step(&a3, 1);
	assert(a3_output[1] == 0 && a3_output[0] == 0);

	ma_set_state(a1, (uint64_t [2]) {1, 0});
	ma_step(a2, a2_size);
	for (size_t i = 0; i < a2_size; i++) {
		assert(a2_output[i][0] == 0);
	}

	for (size_t i = 0; i < a2_size; i++)
		ma_delete(a2[i]);
	ma_delete(a1);
	ma_delete(a3);
	
  printf("pass\n");
	return PASS;
}	


static int custom(void) {
  uint64_t x = 2;
  uint64_t x2 = 10;
  const uint64_t q1 = 0;
  const uint64_t *y[3];
  moore_t *a[3];
  const uint64_t zero[2] = {0, 0};

  a[0] = ma_create_full(10, 10, 10, t_times3, y_plus1, &q1);
  a[1] = ma_create_full(10, 10, 10, t_plus, y_times10, &q1);
  a[2] = ma_create_simple(100, 100, t_shift);
  assert(a[0]);
  assert(a[1]);
  assert(a[2]);
  /*
  Stan automatów:
  0 -> 0 -> 1
  0 -> 0 -> 0
  0, 0 -> 0, 0 -> 0, 0
  */

  y[0] = ma_get_output(a[0]);
  y[1] = ma_get_output(a[1]);
  y[2] = ma_get_output(a[2]);
  assert(y[0] != NULL);
  assert(y[0][0] == 1);
  assert(y[1] != NULL);
  assert(y[1][0] == 0);
  assert(y[2] != NULL);
  assert(y[2][0] == 0 && y[2][1] == 0);

  //Przepisujemy bity z wyjscia a[0] na wyjście a[1]
  assert(ma_connect(a[1], 0, a[0], 0, 10) == 0);
  //Przepisujemy bity z wyjścia a[1] na bity 60-69 wejścia a[1]
  //tym samym rodzielamy wyjście a[1] na 2 części 4-bitową i 6-bitową
  assert(ma_connect(a[2], 60, a[1], 0, 10) == 0);

  ma_set_input(a[0], &x);
  ma_set_input(a[1], zero);
  ma_set_input(a[2], zero);

  ma_step(a, 1);
  ma_step(&(a[1]), 1);
  ma_step(&(a[2]), 1);

  /*
  Stan automatów:
  2 -> 6 -> 7
  7 -> 7 -> 70
  2^61 + 2^62, 4 -> 6, 4 -> 6, 4
  */

  assert(y[0][0] == 7);
  assert(y[1][0] == 70);
  assert(y[2][0] == 6);
  assert(y[2][1] == 4);

  //odłączamy pierwszy bit wejścia automatu 1
  ma_disconnect(a[1], 0, 1);
  //zastępujemy go zeraem (podłaczone bity pozostają nietknięte)
  //teraz na wejściu a[1] powinno znajdowac się 6
  ma_set_input(a[1], &q1);
  ma_step(&(a[1]), 1);
  assert(y[1][0] == 130);
  /*
  Stan automatów:
  2 -> 6 -> 7
  6 -> 13 -> 130
  2^61, 8 -> 6, 4 -> 6, 4
  */

  assert(y[0][0] == 7);
  assert(y[1][0] == 130);
  assert(y[2][0] == 6);
  assert(y[2][1] == 4);

  ma_set_input(a[0], &x2);
  ma_step(a, 3);

  assert(y[0][0] == 31);
  assert(y[1][0] == 190);
  assert(y[2][0] == 2);
  assert(y[2][1] == 8);
  /*
  Stan automatów:
  10 -> 30 -> 31
  (nieistotne (31)) -> 19 -> 190
  (niestotne) -> 2, 8 -> 2, 8
  */


  ma_delete(a[0]);
  ma_delete(a[1]);
  ma_delete(a[2]);
  puts("PASS");
  return PASS;
}

void t_one(uint64_t *next_state, uint64_t const *input,
           uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

void y_one(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] + 1;
}

static void t_two(uint64_t *next_state, uint64_t const *input,
                  uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] ^ input[0];
}

// Testuje jeden automat wykonujący jakieś dodawania.
static int one(void) {
    const uint64_t q1 = 1, x3 = 3, *y;


    moore_t *a = ma_create_full(64, 64, 64, t_one, y_one, &q1);
    assert(a);


    y = ma_get_output(a);
    assert(y != NULL);


    assert(ma_set_input(a, &x3) == 0);


    assert(y[0] == 2);


    assert(ma_step(&a, 1) == 0);


    assert(y[0] == 5);


    assert(ma_step(&a, 1) == 0);


    assert(y[0] == 8);


    assert(ma_set_input(a, &q1) == 0);


    assert(ma_set_state(a, &x3) == 0);


    assert(y[0] == 4);


    assert(ma_step(&a, 1) == 0);


    assert(y[0] == 5);


    assert(ma_step(&a, 1) == 0);


    assert(y[0] == 6);

    ma_delete(a);
    puts("pass");
    return PASS;
}

// Testuje dwa automaty tworzące dwubitowy licznik binarny.
static int two(void) {
  uint64_t x = 1;
  const uint64_t *y[2];
  moore_t *a[2];

  a[0] = ma_create_simple(1, 1, t_two);
  a[1] = ma_create_simple(1, 1, t_two);
  assert(a[0]);
  assert(a[1]);

  y[0] = ma_get_output(a[0]);
  y[1] = ma_get_output(a[1]);
  assert(y[0] != NULL);
  assert(y[1] != NULL);

  // Na początku licznik ma wartość 00.
  assert(ma_set_input(a[0], &x) == 0);
  assert(ma_connect(a[1], 0, a[0], 0, 1) == 0);
  assert(y[1][0] == 0 && y[0][0] == 0);

  // Po jednym kroku licznik ma wartość 01.
  assert(ma_step(a, 2) == 0);
  assert(y[1][0] == 0 && y[0][0] == 1);

  // Po dwóch krokach licznik ma wartość 10.
  assert(ma_step(a, 2) == 0);
  print_automat(a[0]);
  print_automat(a[1]);
  assert(y[1][0] == 1 && y[0][1] == 0);

  // Po trzech krokach licznik ma wartość 11.
  assert(ma_step(a, 2) == 0);
  assert(y[1][0] == 1 && y[0][0] == 1);

  // Po czterech krokach licznik ma wartość 00.
  assert(ma_step(a, 2) == 0);
  assert(y[1][0] == 0 && y[0][0] == 0);
  assert(ma_step(a, 2) == 0);

  // Po pięciu krokach licznik ma wartość 01.
  assert(y[1][0] == 0 && y[0][0] == 1);

  // Po rozłączeniu automatów starczy bit licznika przestaje się zmieniać.
  assert(ma_disconnect(a[1], 0, 1) == 0);
  x = 0;
  assert(ma_set_input(a[1], &x) == 0);
  assert(ma_step(a, 2) == 0);
  assert(y[1][0] == 0 && y[0][0] == 0);
  assert(ma_step(a, 2) == 0);
  assert(y[1][0] == 0 && y[0][0] == 1);

  ma_delete(a[0]);
  ma_delete(a[1]);
  puts("pass");
  return PASS;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
// Błąd alokacji jest zgłaszany raz. Druga próba powinna się udać.
static unsigned long alloc_fail_test(void) {
  printf("hey");
  const uint64_t q1 = 1;
  unsigned long visited = 0;
  moore_t *maf, *mas;

  errno = 0;
  if ((maf = ma_create_full(64, 64, 64, t_one, y_one, &q1)) != NULL)
    visited |= V(1, 0);
  else if (errno == ENOMEM &&
           (maf = ma_create_full(64, 
            64, 64, t_one, y_one, &q1)) != NULL)
    visited |= V(2, 0);
  else
    return visited |= V(4, 0); // To nie powinno się wykonać.

  errno = 0;
  if ((mas = ma_create_simple(1, 1, t_two)) != NULL)
    visited |= V(1, 1);
  else if (errno == ENOMEM && (mas = ma_create_simple(1, 1, t_two)) != NULL)
    visited |= V(2, 1);
  else
    return visited |= V(4, 1); // To nie powinno się wykonać.

  ma_delete(maf);
  ma_delete(mas);

  return visited;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory_test(unsigned long (* test_function)(void)) {
  memory_test_data_t *mtd = get_memory_test_data();

  unsigned fail = 0, pass = 0;
  mtd->call_total = 0;
  mtd->fail_counter = 1;
  while (fail < 3 && pass < 3) {
    mtd->call_counter = 0;
    mtd->alloc_counter = 0;
    mtd->free_counter = 0;
    mtd->function_name = NULL;
    unsigned long visited_points = test_function();
    if (mtd->alloc_counter != mtd->free_counter ||
        (visited_points & 0444444444444444444444UL) != 0) {
      fprintf(stderr,
              "fail_counter %u, alloc_counter %u, free_counter %u, "
              "function_name %s, visited_point %lo\n",
              mtd->fail_counter, mtd->alloc_counter, mtd->free_counter,
              mtd->function_name, visited_points);
      ++fail;
    }
    if (mtd->function_name == NULL)
      ++pass;
    else
      pass = 0;
    mtd->fail_counter++;
  }
if(mtd->call_total > 0 && fail == 0) puts("pass");
else puts("fail (test na niepowodzenie alokacji)");
  return mtd->call_total > 0 && fail == 0 ? PASS : FAIL;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory(void) {
  memory_tests_check();
  return memory_test(alloc_fail_test);
}

void t_weird(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] + (input[1] << 14);
  next_state[1] = old_state[1] + (input[0] << 14) + (input[1] >> 50);
  next_state[2] = old_state[2] + (input[2] >> 50) + (input[0] >> 50) ^ input[1];
}

void t_weird2(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[1] + (input[1] << 13);
  next_state[1] = old_state[2] + (input[0] << 13) + (input[1] >> 50);
  next_state[2] = old_state[0] * 7 + (input[2] ^ 123879) + 14;
}

void t_weird3(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[1] + (input[1] << 12);
  next_state[1] = old_state[0] + (input[0] << 12) + (input[1] >> 2) + 123897;
  next_state[2] = old_state[0] + (input[2] >> 2) + (input[0] >> 2) ^ input[1] * 4;
}

void y_xor(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] & state[1];
  output[1] = state[2] ^ state[0];
  output[2] = state[1] ^ state[2];
}

// Ignacy Boehlke
// Użycie: includujemy gdzieś w ma_example.c i dodajemy TEST(connection_stress) do
// test_list.
//
// Sprawdzarce można włączyć więcej wypisywania za pomocą zmiennej środowiskowej:
// $ PRINT_INFO=1 ./ma_example connection_stress
//
// W testach są trzy automaty, dwa mające tylko wyjście (wyjściowe) i jeden, który
// przepisuje swoje wejście na wyjście (główny). W ramach zapytania podłączamy losowy
// zakres wyjść z wyjściowych automatów do wejść głownego, odłączamy losowy zakres wejść
// od głównego, albo usuwamy któryś z automatów i tworzymy go na nowo. Po każdym zapytaniu
// sprawdzamy, czy stan na wyjściu głównego automatu się odpowiednio zmienił.
// Sposób obsługiwania przez bibliotekę bitów poza zakresem nie powinien mieć wpływu na
// wynik.

#include "ma.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PASS 0
#define FAIL 1

#define BITC_TO_BYTEC(x) ((x+7)/8)
#define BITC_TO_64C(x) ((x+63)/64)

void output_simple(uint64_t *output, uint64_t const *state, size_t m, size_t s) {
	(void)m, (void)s;
	if (state[0] & 1)
		memset(output, -1, BITC_TO_BYTEC(m));
	else
		memset(output,  0, BITC_TO_BYTEC(m));
}

void flip(uint64_t *next_state, uint64_t const *input,
          uint64_t const *state, size_t n, size_t s) {
	(void)input, (void)n, (void)s;
	next_state[0] = !state[0];
}

void my_identity(uint64_t *output, uint64_t const *state, size_t m, size_t s) {
	(void)m;
	memcpy(output, state, BITC_TO_BYTEC(s));
}

void mirror_input(uint64_t *next_state, uint64_t const *input,
                  uint64_t const *state, size_t n, size_t s) {
	(void)state, (void)n, (void)s;
	memcpy(next_state, input, BITC_TO_BYTEC(n));
}

static void pad_with_zeros(uint64_t *arr, size_t size) {
	if (size % 64)
		arr[size / 64] &= ((uint64_t)1 << (size % 64)) - 1;
}

static bool bitset_equal(uint64_t *a, uint64_t *b, size_t s) {
	pad_with_zeros(a, s);
	pad_with_zeros(b, s);
	return !memcmp(a, b, BITC_TO_BYTEC(s));
}

static void bitset_or(uint64_t *res, uint64_t *a, uint64_t *b, size_t s) {
	for (size_t i = 0; i < BITC_TO_64C(s); ++i)
		res[i] = a[i] | b[i];
	pad_with_zeros(res, s);
}

static void bitset_xor(uint64_t *res, uint64_t *a, uint64_t *b, size_t s) {
	for (size_t i = 0; i < BITC_TO_64C(s); ++i)
		res[i] = a[i] ^ b[i];
	pad_with_zeros(res, s);
}

static void set_bit(uint64_t *arr, size_t i, bool value) {
	arr[i / 64] |= ((uint64_t)value << (i % 64));
	arr[i / 64] &= ~((uint64_t)!value << (i % 64));
}

static size_t rd(size_t a, size_t b) {
	return a + (size_t)rand() % (b - a + 1);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void print_bits(uint64_t *arr, size_t s) {
	for (size_t i = 0; i < s; ++i)
		fputc('0' + ((arr[i / 64] >> (i % 64)) & 1), stderr);
	fputc('\n', stderr);
}

#define PRINT_BITS(comment, x, len) do if (len <= 100) {               \
	fprintf(stderr, "%s:\n", comment);                                 \
	print_bits(x, len);                                                \
} while(0)

#define INFO(x...) if (PRINT_INFO) fprintf(stderr, x)

#define CALL(func) do {                                                \
	int _r = func;                                                     \
	if (!_r) break;                                                    \
	fprintf(stderr, "%s at %s:%d: %s\n", #func, __FILE__, __LINE__,    \
	                                     strerror(errno));             \
	return FAIL;										               \
} while(0)

static bool PRINT_INFO = 1;

uint64_t *zero_array, *connected, *connected_to_1, *buff, *buff2, *buff3;
static void conn_allocate(size_t s) {
	zero_array     = (uint64_t*)malloc(s);
	memset(zero_array, 0, s);
	connected      = (uint64_t*)malloc(s);
	connected_to_1 = (uint64_t*)malloc(s);
	buff           = (uint64_t*)malloc(s);
	buff2          = (uint64_t*)malloc(s);
	buff3          = (uint64_t*)malloc(s);
}

static void conn_free() {
	free(zero_array    );
	free(connected     );
	free(connected_to_1);
	free(buff          );
	free(buff2         );
	free(buff3         );
}

static uint64_t val_source[2] = {0, 1};
static moore_t *create_source(size_t *size, size_t max_size, bool value) {
	*size = rd(1, max_size);
  errno=0;
	moore_t *m = ma_create_full(0, *size, 1, flip, output_simple, &val_source[value]);
  if (m==NULL) assert(errno == EINVAL);
	assert(m);
	return m;
}

static moore_t *create_main(size_t *size, size_t max_size, const uint64_t **output) {
	*size = rd(1, max_size);
	moore_t *m = ma_create_full(*size, *size, *size, mirror_input, my_identity, zero_array);
	assert(m);
	ma_set_input(m, zero_array);
	*output = ma_get_output(m);
	return m;
}

typedef struct conn_test_config {
	size_t run_count;
	size_t query_count;
	size_t max_size;
} conn_test_config_t;

static const conn_test_config_t conn_tests[] = {
	{100, 100000, 10},
	{100, 100000, 100},
	{100, 1000, 10000},
	{10, 100, 1000000},
};

static int run_connection_test(size_t run_id, size_t query_count, size_t max_size) {
	size_t s[2], mains;
	moore_t *m[3], *main;
	const uint64_t *out;

	m[0] = create_source(&s[0], max_size, 0);
	m[1] = create_source(&s[1], max_size, 1);
	main = m[2] = create_main(&mains, max_size, &out);

	memset(connected, 0, BITC_TO_BYTEC(mains));
	memset(connected_to_1, 0, BITC_TO_BYTEC(mains));

	for (size_t q = 1; q <= query_count; ++q) {
		INFO("Query %lu: ", q);
		int action = rd(0, 29);

		size_t lmain = rd(0, mains - 1);
		if (10 <= action) {
			int source = rd(0, 1);
      
			size_t lsource = rd(0, s[source] - 1);
			size_t num = rd(1, MIN(s[source] - lsource, mains - lmain));
      printf("%lu, %lu, %lu\n", source, lsource, num);
			INFO("connecting outputs %lu-%lu from m%d to inputs %lu-%lu\n",
					lsource, lsource + num - 1, source, lmain, lmain + num - 1);

			CALL(ma_connect(main, lmain, m[source], lsource, num));
        //print_automat(main);
			for (size_t i = 0; i < num; ++i) {
				set_bit(connected, lmain + i, 1);
				set_bit(connected_to_1, lmain + i, source);
			}
		} else if (1 <= action) {
			size_t num = rd(1, mains - lmain);

			INFO("disconnecting inputs %lu-%lu\n", lmain, lmain + num - 1);
      
			for (size_t i = 0; i < num; ++i) {
				set_bit(connected, lmain + i, 0);
				set_bit(connected_to_1, lmain + i, 0);
			}
			CALL(ma_disconnect(main, lmain, num));
      
			ma_set_input(main, zero_array);
      //print_automat(main);
		} else {
			int mid = rd(0, 2);
			INFO("deleting and creating machine %d\n", mid);
      //print_automat(m[1]);
      //print_automat(m[2]);
      //printf("Deleting machine %d\n", mid);
			ma_delete(m[mid]);
      //printf("deleted");
      //print_automat(m[2]);
			if (mid < 2) {
				m[mid] = create_source(&s[mid], max_size, mid);
				if (mid == 1) {
					bitset_xor(connected, connected, connected_to_1, mains);
					memset(connected_to_1, 0, BITC_TO_BYTEC(mains));
				} else {
					memcpy(connected, connected_to_1, BITC_TO_BYTEC(mains));
				}
			} else {
				main = m[mid] = create_main(&mains, max_size, &out);
				memset(connected, 0, BITC_TO_BYTEC(mains));
				memset(connected_to_1, 0, BITC_TO_BYTEC(mains));
			}
			ma_set_input(main, zero_array);
      //print_automat(m[1]);
		}

		// checking if the new connections/disconnections were handled correctly

		uint64_t *output = buff, *inverted = buff2, *ored = buff3;

		CALL(ma_step(&main, 1));    // we update values on main
		memcpy(output, out, BITC_TO_BYTEC(mains));

		CALL(ma_step(m, 2));	    // we flip the values on source machines
    CALL(ma_step(m, 3));		// we update main again and flip the values back
		memcpy(inverted, out, BITC_TO_BYTEC(mains));



		bitset_or(ored, output, inverted, mains);
		if (!bitset_equal(ored, connected, mains) ||
				!bitset_equal(output, connected_to_1, mains)) {
			fprintf(stderr, "Failed on run %lu, query %lu\n", run_id, q);
			PRINT_BITS("Which inputs should be connected", connected, mains);
			PRINT_BITS("Which inputs libma says are connected", ored, mains);
			PRINT_BITS("Which inputs should be connected to m1", connected_to_1, mains);
			PRINT_BITS("Which inputs libma says are connected to m1", output, mains);
			PRINT_BITS("Which inputs libma says are connected to m0", inverted, mains);
			for (int i = 0; i < 3; ++i)
				ma_delete(m[i]);
			return FAIL;
		}
	}
	
	for (int i = 0; i < 3; ++i)
		ma_delete(m[i]);
	return PASS;
}

#ifndef SIZE
#define SIZE(X) (sizeof (x) / sizeof (x[0]))
#endif

static int connection_stress(void) {
	srand(~2137);
	if (getenv("PRINT_INFO"))
		PRINT_INFO = (bool)atol(getenv("PRINT_INFO"));
	size_t max_size = 0;
	for (size_t i = 0; i < SIZE(conn_tests); ++i)
		max_size = MAX(max_size, conn_tests[i].max_size);

	conn_allocate(BITC_TO_BYTEC(max_size));

	for (size_t group = 0; group < SIZE(conn_tests); ++group) {
		conn_test_config_t cfg = conn_tests[group];
		fprintf(stderr, "Running test group %lu – %lu runs, %lu queries per run, %lu max signal count\n", group + 1, cfg.run_count, cfg.query_count, cfg.max_size);
		for (size_t run = 0; run < cfg.run_count; ++run) {
			INFO("Run %lu\n\n", run);
			if (run_connection_test(run, cfg.query_count, cfg.max_size) == FAIL)
				return FAIL;
		}
	}

	conn_free();
	return PASS;
}


static int custom2(void) {
  const uint64_t q0[3] = {0, 0, 0};
  const uint64_t q_noise[3] = {120ll*315*213*1273+123, 91278381953132213, 1982357853123483726};
  const uint64_t q_noise2[3] = {
    q_noise[0] ^ q_noise[1] + q_noise[2],
    q_noise[1] ^ q_noise[2] + q_noise[0] * 12,
    q_noise[2] + q_noise[0] * 7 + 2 * q_noise[1]
  };
  const uint64_t q_noise3[3] = {
    q_noise[0] + q_noise2[1] * 2 + q_noise[2] * 3,
    q_noise2[1] + q_noise[2] * 4 + q_noise[0] * 5,
    q_noise[2] + q_noise[0] * 6 + q_noise2[1] * 7
  };

  const size_t N = 10; // Zapomniałem, że to istnieje, więc lepiej tego nie zmieniać

  // create the automatas

  moore_t *a[N];
  const uint64_t* o[N];
  a[0] = ma_create_full(192, 192, 192, t_weird, y_xor, q_noise);
  a[1] = ma_create_full(192, 192, 192, t_weird2, y_xor, q_noise2);
  a[2] = ma_create_full(192, 192, 192, t_weird3, y_xor, q_noise3);
  for (size_t i = 3; i < N; i++) {
    a[i] = ma_create_simple(192, 192, t_weird3);
  }

  for(size_t i = 0; i < N; i++) {
    assert(a[i] != NULL);
  }

  for(size_t i = 0; i < N; i++) {
    o[i] = ma_get_output(a[i]);
    assert(o[i] != NULL);
  }

  uint64_t operation = 1;
  scanf("%llu", &operation);
  for(size_t i = 0; operation != 0; ++i) {
    operation++;
    if(operation < 10){
      ma_delete(a[operation]);
      a[operation] = ma_create_simple(192, 192, t_weird);
      assert(a[operation] != NULL);
      o[operation] = ma_get_output(a[operation]);
      assert(o[operation] != NULL);
    }
    else if(operation < 20){
      operation -= 10;
      ma_step(&a[operation], 1);
    }
    else if(operation < 60){
      operation -= 20;
      if(operation % 4 == 0){
        ma_set_input(a[operation/4], q_noise);
      }
      else if(operation % 4 == 1){
        ma_set_input(a[operation/4], q_noise2);
      }
      else if(operation % 4 == 2){
        ma_set_input(a[operation/4], q_noise3);
      }
      else{
        ma_set_input(a[operation/4], q0);
      }
    }
    else{
      size_t automata_idx = operation % 10;
      operation /= 10;

      size_t num_bits = operation % 192 + 1;
      operation /= 192;

      size_t out_idx = operation % 192;
      operation /= 192;

      size_t in_idx = operation % 192;
      operation /= 192;

      if(in_idx + num_bits > 192){
        num_bits = 192 - in_idx;
      }
      if(out_idx + num_bits > 192){
        num_bits = 192 - out_idx;
      }

      operation %= 10;

      ma_connect(a[automata_idx], in_idx, a[operation], out_idx, num_bits);
    }

    ma_step(a, N);
    for(int j = 0; j < N; j++){
      printf("%llu %llu %llu\n", o[j][0], o[j][1], o[j][2]);
    }

    scanf("%llu", &operation);
  }

  for(size_t i = 0; i < N; i++){
    ma_delete(a[i]);
    a[i] = NULL;
  }

  return PASS;
}

/** URUCHAMIANIE TESTÓW **/

typedef struct {
  char const *name;
  int (*function)(void);
} test_list_t;

#define TEST(t) {#t, t}

static const test_list_t test_list[] = {
  TEST(one),
  TEST(two),
  TEST(memory),
  TEST(disruptor),
  TEST(custom),
  TEST(custom2),
  TEST(connection_stress)
};

static int do_test(int (*function)(void)) {
  printf("not yay");
  int result = function();
  puts("quite long magic string");
  return result;
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    for (size_t i = 0; i < SIZE(test_list); ++i)
      if (strcmp(argv[1], test_list[i].name) == 0)
        return do_test(test_list[i].function);

  fprintf(stderr, "Użycie:\n%s nazwa_testu\n", argv[0]);
  return WRONG_TEST;
}