// Mateusz Kussowski
// testy do zadania z AKSO
// testy te niestey nie u każdego muszą zwracać ten sam wynik
// zależy to od zachowania innych automatow po usunięciu automatu źródłowego
// polecam korzystać wtedy ze starego wejścia, tzn ostatniego znanego wyjścia
// automatu źródłowego, tzn. niczego sztucznie nie zerować, 
// wtedy wynik powinien być zawsze ten sam

// jeżeli jednak dane wejściowe są >= 10 to wynik powienien być zawsze ten sam

// Jak tego użyć
// 1. dodajemy do ma_example.c:
//   #include "tests/custom2.c"
//   ...
//   TEST(custom2),
//   ...
// 2. kompilujemy wszystko (np. do ma_example)
// 3. uruchamiamy ./ma_example custom2 < "nazwa_testu.in" > "nazwa_testu.out"

// możemy porownac koncowki pliku lub hashe

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

#include <stdio.h>
#include <stdlib.h>

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
    ASSERT(a[i] != NULL);
  }

  for(size_t i = 0; i < N; i++) {
    o[i] = ma_get_output(a[i]);
    ASSERT(o[i] != NULL);
  }

  uint64_t operation = 1;
  scanf("%llu", &operation);
  for(size_t i = 0; operation != 0; ++i) {
    operation++;
    if(operation < 10){
      ma_delete(a[operation]);
      a[operation] = ma_create_simple(192, 192, t_weird);
      ASSERT(a[operation] != NULL);
      o[operation] = ma_get_output(a[operation]);
      ASSERT(o[operation] != NULL);
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

