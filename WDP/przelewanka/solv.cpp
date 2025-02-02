#include <bits/stdc++.h>

using namespace std;

map < vector < int > , int > visited;
queue < vector < int >> states;
int n, q1, q2;
vector < int > x, y;
int minim = INT_MAX;

void backtrack_bfs() {
  vector < int > curr;
  int moves = 0;

  //algorytm na zasadzie bfs, z każdego stanu wszystkie osiągalne ale nie osiągnalne w mniejszej liczbie kroków dodaje na koniec kolejki
  //dzieki temu przechodząc po kolejce rozważam wszystkie możliwe stany w kolejności o najmniejszej liczby potrzebnych ruchów
  while (states.size()) {
    curr = states.front();
    states.pop();
    moves = visited[curr];

    if (curr == y) {
      minim = min(minim, moves);
      //ponieważ idziemy po kolei, żaden kolejny stan z kolejki nie będzie osiągany w mniejszej liczbie kroków
      return;
    }

    for (int i = 0; i < n; i++) {
      int mem_i = curr[i];

      //napelniamy i-tą szklankę
      curr[i] = x[i];
      if (!visited[curr]) {
        visited[curr] = moves + 1;
        states.push(curr);
      }

      //opróżniamy i-tą szklankę
      curr[i] = 0;
      if (!visited[curr]) {
        visited[curr] = moves + 1;
        states.push(curr);
      }

      curr[i] = mem_i;
      //przelewamy z i-tej do j-tej szklanki
      for (int j = 0; j < n; j++) {
        if (i == j) continue;

        int mem_j = curr[j];

        //objętość przelanej wody
        int delta = min(curr[i], x[j] - curr[j]);
        curr[j] += delta;
        curr[i] -= delta;

        if (!visited[curr]) {
          visited[curr] = moves + 1;
          states.push(curr);
        }
        curr[i] = mem_i;
        curr[j] = mem_j;
      }
    }

  }
}

int main() {
  cin >> n;

  if (n==0){
    cout << 0;
    return 0;
  }
  bool if_ok = 0;
  int gcd;
  vector < int > state;

  for (int i = 0; i < n; i++) {
    cin >> q1 >> q2;
    x.push_back(q1);
    y.push_back(q2);
    if (q1 == q2 || q2 == 0) if_ok = 1;
    state.push_back(0);
  }

  gcd = 0;
  for (int i = 0; i < n; i++) {
    if (x[i]!=0) {
      if (gcd == 0) gcd=x[i];
      else gcd = __gcd(gcd, x[i]);
    }
  }



  //każda końcowa objętość musi być wielokrotnością nwd wszystkich pojemności aby dało się ją osiągnąć
  for (int i = 0; i < n; i++) {
    if (gcd != 0 && y[i] % gcd != 0) if_ok = 0;
  }

  //któraś szklanka na koniec musi być pełna lub pusta (sprawdzane przy wczytywaniu) + warunek z nwd
  if (if_ok == 0) {
    cout << -1;
    return 0;
  }

  //visited[stan] to minimalna liczba kroków potrzebna do osięgnięcia danego stanu
  visited[state] = 1;

  if (state == y) {
    cout << 0;
    return 0;
  }

  //states to kolejka trzymająca kolejne osiągalne stany, posortowane po minimalnej liczbie ruchów potrzebnych
  states.push(state);

  backtrack_bfs();
  if (minim == INT_MAX) cout << -1;
  else cout << minim - 1;

}