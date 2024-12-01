#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>

int min(int a, int b) {
  if (a <= b)
    return a;
  else
    return b;
}

int max(int a, int b) {
  if (a >= b)
    return a;
  else
    return b;
}

int closest(int n, int *left, int *type, int *dist) { // maksimum z odległości jest minimalne
  int first, last, middle;
  int ans = INT_MAX;
  first = left[1] - 1;
  middle = left[1];
  last = left[2];

  while (last < n) { // gąsienica
    if (type[first] == type[last] || type[middle] == type[last]) {
      last++;
      continue;
    }

    if (type[middle] == type[first]) {
      middle++;
      continue;
    }

    ans = min(ans, max(dist[middle] - dist[first], dist[last] - dist[middle]));

    if (middle < last && type[middle] == type[middle + 1]) {
      middle++;
      continue;
    }

    first = middle;
  }
  return ans;
}

bool diff_types(int k, int l, int m, int *type) { // czy sieci hoteli o nr k, l, m są różne
  if (type[k] == type[l] || type[l] == type[m] || type[k] == type[m])
    return false;
  else
    return true;
}

int furthest(int n, int *left, int *right, int *type, int *dist) { // minumum z odleglosci ma byc maksymalne
  int ans = 0;

  int middle = 1;
  while (middle < n) {
    for (int l = 0; l < 3; l++) { // wszytskie kombinacje skrajnie lewy x skrajnie prawy
      for (int r = 2; r >= 0; r--) {
        if (diff_types(left[l], middle, right[r], type))
          ans = max(
            ans,
            min(dist[middle] - dist[left[l]], dist[right[r]] - dist[middle]));
        // jesli rozważany środek jest poza przedziałem (poczatek, koniec) lub
        // koniec < początek wartość minimum z odległości będzie ujemna, 
        // czyli bez znaczenia
      }
    }
    middle++;
  }
  return ans;
}

bool extremes(int n, int * left, int * right, int * type) { 
  // numery trzech skrajnych z lewej i trzech skrajnych z prawej hoteli różnych sieci
  // zwraca true tylko jeśli są co najmniej 3 różne sieci, w przeciwnym wypadku false

  // lewe
  left[0] = 0;
  int i = 1;
  while (i < n && type[i] == type[left[0]])
    i++;
  if (i == n) { // wszystkie są tej samej sieci
    return false;
  }
  left[1] = i;
  while (i < n && (type[i] == type[left[0]] || type[i] == type[left[1]]))
    i++;
  if (i == n) { // są tylko 2 sieci
    return false;
  }
  left[2] = i;

  // prawe
  right[2] = n - 1;
  i = n - 2;
  while (type[i] == type[right[2]])
    i--;
  right[1] = i;
  while (type[i] == type[right[2]] || type[i] == type[right[1]])
    i--;
  right[0] = i;

  return true;
}

int main() {
  int n;
  int * type;
  int * dist;
  int * left;
  int * right;

  assert(scanf("%d", & n));
  dist = (int * ) malloc((long unsigned int) n * sizeof(int)); // odleglosc od poczatku dla hotelu z numerem i;
  type = (int * ) malloc((long unsigned int) n * sizeof(int)); // siec hotelu z numerem i

  for (int i = 0; i < n; i++) {
    assert(scanf("%d %d", & type[i], & dist[i]));
  }

  left = (int * ) malloc(3 * sizeof(int));
  right = (int * ) malloc(3 * sizeof(int));

  if (extremes(n, left, right, type)){ //jesli są co najmniej 3 różne typy
    printf("%d %d\n", closest(n, left, type, dist), furthest(n, left, right, type, dist));
  }
  else {
    printf("0 0\n");
  }

  free(left);
  free(right);
  free(dist);
  free(type);

  return 0;
}