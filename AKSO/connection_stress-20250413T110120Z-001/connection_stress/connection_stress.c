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

static bool PRINT_INFO = 0;

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
	moore_t *m = ma_create_full(0, *size, 1, flip, output_simple, &val_source[value]);
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

			INFO("connecting outputs %lu-%lu from m%d to inputs %lu-%lu\n",
					lsource, lsource + num - 1, source, lmain, lmain + num - 1);

			CALL(ma_connect(main, lmain, m[source], lsource, num));

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
		} else {
			int mid = rd(0, 2);
			INFO("deleting and creating machine %d\n", mid);
			ma_delete(m[mid]);
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
