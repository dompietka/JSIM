#include <cstdlib>

#include "kol.h"

using namespace std;

namespace {

typedef struct lista {
  interesant *head, *tail;
} lista;

lista create_list() {
  lista res;
  res.head = (interesant*)malloc(sizeof(interesant));
  res.tail = (interesant*)malloc(sizeof(interesant));
  res.head->l1 = NULL;
  res.head->l2 = res.tail;
  res.tail->l1 = NULL;
  res.tail->l2 = res.head;
  return res;
}

bool isempty(const lista l) {
  return l.head->l2 == l.tail;
}

interesant *front(const lista l) {
  return l.head->l2;
}

interesant *back(const lista l) {
  // Przerobione z front
  return l.tail->l2;
}

void link(interesant *el, interesant *old, interesant *nw) {
  if (el) {
    if (el->l1 == old) {
      el->l1 = nw;
    } else {
      el->l2 = nw;
    }
  }
}

// Nieużytek
/*void push_front(lista &l, int x) {
  interesant *el = (interesant*)malloc(sizeof(interesant));
  el->val = x;
  el->l1 = l.head;
  el->l2 = l.head->l2;
  link(l.head->l2, l.head, el);
  l.head->l2 = el;
}*/

void push_back(lista &l, int x) {
  // Przerobione z push_front
  interesant *el = (interesant*)malloc(sizeof(interesant));
  el->val = x;
  el->l1 = l.tail;
  el->l2 = l.tail->l2;
  link(l.tail->l2, l.tail, el);
  l.head->l2 = el;
}

void remove_interesant(interesant *el) {
  link(el->l1, el, el->l2);
  link(el->l2, el, el->l1);
  free(el);
}

void pop_front(lista &l) {
  if (!isempty(l)) {
    remove_interesant(l.head->l2);
  }
}

void reverse(lista &l) {
  interesant *tmp = l.head;
  l.head = l.tail;
  l.tail = tmp;
}

// Nieuzytek
/*void append(lista &l1, lista &l2) {
  interesant *b = l1.tail;
  interesant *f = l2.head;
  link(b->l2, b, f->l2);
  link(f->l2, f, b->l2);
  b->l2 = f;
  f->l2 = b;
  l1.tail = l2.tail;
  l2.head = f;
  l2.tail = b;
}*/

vector<lista> kolejki;

int akt_numerek;

}

void otwarcie_urzedu(int m) {
  kolejki.resize(m);
  for (int i = 0; i < m; ++i)
    kolejki[i] = create_list();
  akt_numerek = 0;
}

interesant *nowy_interesant(int k) {
  push_back(kolejki[k], akt_numerek++);
  return back(kolejki[k]);
}

// TODO
// int numerek(interesant *i);

interesant *obsluz(int k) {
  if (isempty(kolejki[k]))
    return NULL;
  interesant *i = front(kolejki[k]);
  pop_front(kolejki[k]);
  return i;
}

// TODO
// void zmiana_okienka(interesant *i, int k);

// TODO
// void zamkniecie_okienka(int k1, int k2);

// TODO 
// std::vector<interesant *> fast_track(interesant *i1, interesant *i2);

void naczelnik(int k) {
  reverse(kolejki[k]);
}

// TODO
// vector<interesant *> zamkniecie_urzedu();
