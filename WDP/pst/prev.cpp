#include <bits/stdc++.h>

#include "prev.h"

using namespace std;
#define int_log 32
#define left 0
#define right 1

struct node {
  int value; //najwiekszy dotychczasowy indeks na tym zakresie 
  int min_range;
  int max_range; //^^  przedzial obejmowany tym wierzcholkiem
  shared_ptr < node > left_son; //wskaźnik na lewego syna
  shared_ptr < node > right_son; //wskaźnik na prawego syna
};

vector < shared_ptr < node >> tree; //korzeń drzewa

void add2root() {
  tree.push_back(make_shared < node > (node {
    (int) tree.size(), INT_MIN, INT_MAX, nullptr, nullptr
  }));
}

int middle(shared_ptr < node > curr) {
  //znajduje środek zakresu
  long long h = ((long long) curr -> max_range - (long long) curr -> min_range) / 2;
  int mid = (int) h + curr -> min_range;
  return mid;
}

bool direction(shared_ptr < node > curr, int value) {
  //dla wartości i zakresu wierzchołka zwraca kierunek syna, do którego należy ta wartość
  int mid = middle(curr);

  if (value <= mid) return left;
  else return right;
}

void add_new_son(shared_ptr < node > father, bool dir) {
  //dodaje do wierzchołka dziecko po odpowiedniej stronie
  int mid = middle(father);

  if (dir == left)
    father -> left_son = make_shared < node > (node {
      father -> value, father -> min_range, mid, nullptr, nullptr
    });
  else
    father -> right_son = make_shared < node > (node {
      father -> value, mid + 1, father -> max_range, nullptr, nullptr
    });
}

shared_ptr < node > son(shared_ptr < node > father, bool dir) {
  //zwraca wskaźnik na syna po odpowiedniej stronie
  if (father == nullptr)
    return nullptr;
  if (dir == left)
    return father -> left_son;
  else
    return father -> right_son;
}
void link_prevs_son(shared_ptr < node > father, shared_ptr < node > prev, bool dir) {
  //podłącza jako dziecko dziecko analogicznego wierzchołka z poprzedniego stanu, po odpowiedniej stronie
  if (dir == left)
    father -> left_son = son(prev, dir);
  else
    father -> right_son = son(prev, dir);
}

void init(const std::vector < int > & seq) {
  add2root();

  for (auto el: seq) {
    pushBack(el);
  }
}

int query(shared_ptr < node > curr, int left_bound, int right_bound) {
  //rekurencyjne przejscie po drzewie
  if (curr == nullptr)
    return 0;

  if (curr -> min_range > right_bound || curr -> max_range < left_bound) //zakres wierzchołka rozłączny z poszukiwanym
    return 0;

  if (curr -> max_range <= right_bound && curr -> min_range >= left_bound) // zakres wierzchołka w pełni się zawiera w poszukiwanym
    return curr -> value;

  //zakres wierzchołka częściowo pokrywa poszukiwany zakres
  return max(query(curr -> left_son, left_bound, right_bound), query(curr -> right_son, left_bound, right_bound));

}

int prevInRange(int i, int lo, int hi) {
  shared_ptr < node > curr = tree[i + 1];

  return query(curr, lo, hi) - 1; //w korzeniu numeracja jest przesunięta o 1, stąd -1
}

void pushBack(int value) {
  add2root(); //dodaje wartość do korzenia

  shared_ptr < node > curr = tree[(int) tree.size() - 1]; //wskaźnik na korzeń drzewa dodawango elementu
  shared_ptr < node > prev = tree[(int) tree.size() - 2]; //wskaźnik na korzeń drzewa poprzednio dodanego elementu

  bool dir; //kierunek

  for (int k = 0; k < int_log; k++) {
    dir = direction(curr, value);
    link_prevs_son(curr, prev, !dir); //tam gdzie nie ma wracam do poprzedniego stanu drzewa
    add_new_son(curr, dir); //tam gdzie val jest dodaje nowe dziecko

    curr = son(curr, dir); //przenoszę się do kolejnego poziomu drzewa
    prev = son(prev, dir); //także w poprzednim stanie drzewa

  }

}

void done() {
  tree.clear();
}