#include <bits/stdc++.h>
#include "kol.h"

using namespace std;

/*
struct interesant {
    int id; //numerek
    interesant *l1, *l2; //sąsiadujący z nim 
};
*/

struct lista { // lista dwustronna
  interesant * poczatek, * koniec;
};

lista nowa_lista() { // tworzenie strażników nowej listy
  lista l;
  l.poczatek = (interesant * ) malloc(sizeof(interesant));
  l.koniec = (interesant * ) malloc(sizeof(interesant));
  l.poczatek -> l1 = NULL;
  l.poczatek -> l2 = l.koniec;
  l.koniec -> l1 = NULL;
  l.koniec -> l2 = l.poczatek;
  return l;
}

bool puste(lista & l) { // zwraca true jesli pusta
  return l.poczatek -> l2 == l.koniec;
}

interesant * pierwszy(lista & l) { // zwraca wskaźnik na pierwszy element
  return l.poczatek -> l2;
}

interesant * ostatni(lista & l) { // zwraca wskaźnik na ostatni element
  return l.koniec -> l2;
}

void przelacz(interesant * el, interesant * stary, interesant * nowy) { // przełącza wskaźnik el wskazujący na stary na nowy
  if (el == NULL) 
    return;
  if (el -> l2 == stary)
    el -> l2 = nowy;
  else
    el -> l1 = nowy;
}

void dodaj(lista & l, interesant * el) { // dodaje elemnt na koniec listy
  if (el == NULL) 
    return;
  el -> l1 = l.koniec;
  el -> l2 = l.koniec -> l2;
  przelacz(l.koniec -> l2, l.koniec, el);
  l.koniec -> l2 = el;
}

void usun(interesant * el) { // usuwa dany element z listy
  przelacz(el -> l1, el, el -> l2);
  przelacz(el -> l2, el, el -> l1);
}

void usun_pierwszy(lista & l) { // usuwa pierwszy element z listy ("pop_front")
  if (!puste(l)) 
    usun(l.poczatek -> l2);
}

void odwroc(lista & l) { // odwraca kolejnosc listy
  interesant * pom = l.poczatek;
  l.poczatek = l.koniec;
  l.koniec = pom;
}

vector < lista > kolejki; // globalny wektor kolejek
int nr; // globalny nr ktory dostanie kolejny interesant

void wypisz() { // pomocnicze do debuggowania
  for (int i = 0; i < (int) kolejki.size(); i++) {
    if (puste(kolejki[i])) cout << "puste " << i << endl;
    else {
      cout << kolejki[i].poczatek -> l2 -> id << endl;
    }
  }
  cout << endl;
  return;
}

void otwarcie_urzedu(int m) {
  kolejki.resize(m);
  for (int i = 0; i < m; ++i)
    kolejki[i] = nowa_lista();
  nr = 0;
}

interesant * nowy_interesant(int k) {
  interesant * el = (interesant * ) malloc(sizeof(interesant));
  el -> id = nr++;
  dodaj(kolejki[k], el);
  return ostatni(kolejki[k]);
}

int numerek(interesant * i) {
  return i -> id;
}

void naczelnik(int k) {
  odwroc(kolejki[k]);
}

interesant * obsluz(int k) {
  if (puste(kolejki[k]))
    {
      cout << "bylo puste " << k << endl;;
      return NULL;
    }
  wypisz();
  interesant * i = pierwszy(kolejki[k]);
  usun_pierwszy(kolejki[k]);
  
  return i;
}

void zmiana_okienka(interesant * i, int k) {
  usun(i);
  dodaj(kolejki[k], i);
}

void scal(lista & l1, lista & l2) { // dokleja l1 na koniec l2
  interesant * k = l1.koniec;
  interesant * p = l2.poczatek;

  przelacz(k -> l2, k, p -> l2);
  przelacz(p -> l2, p, k -> l2);

  k -> l2 = p;
  p -> l2 = k;

  l1.koniec = l2.koniec;
  l2.poczatek = p;
  l2.koniec = k;
}

void zamkniecie_okienka(int k1, int k2) {
  scal(kolejki[k2], kolejki[k1]);
}

vector < interesant * > fragment(interesant * poprzedni, interesant * obecny, interesant * ostatni, bool kierunek) {
  //fragment listy od poprzedniego do konca listy lub ostatniego, w okreslonym kierunku
  vector < interesant * > wynik = {
    poprzedni
  };

  while (obecny != ostatni && obecny != NULL) {
    wynik.push_back(obecny);

    if (obecny -> l2 == poprzedni) {
      poprzedni = obecny;
      if (kierunek == 1)
        obecny = obecny -> l1;
      else
        obecny = obecny -> l2;
    } else {
      poprzedni = obecny;
      obecny = obecny -> l2;
    }

  }
  wynik.push_back(obecny);
  return wynik;
}

vector < interesant * > fast_track(interesant * i1, interesant * i2) {
  vector < interesant * > podkolejka;

  if (i1 == NULL || i2 == NULL) //jesli ktoregos nie ma
    return podkolejka;

  podkolejka.push_back(i1);

  if (i1 == i2) //jesli sa tym samym elementem
    return podkolejka;

  podkolejka = fragment(i1, i1 -> l2, i2, 0);
  if (podkolejka.back() == NULL) // w kierunku 0 doszlismy do konca zamiast do ostatniego elementu, czyli to był zły kierunek
    podkolejka = fragment(i1, i1 -> l1, i2, 1);

  for (interesant * el: podkolejka)
    usun(el);
  return podkolejka;

}

vector < interesant * > zamkniecie_urzedu() {
  vector < interesant * > pozostali;

  for (lista l: kolejki) {
    while (!puste(l)) {
      pozostali.push_back(pierwszy(l));
      usun_pierwszy(l);
    }

    //czyszczenie pamieci ustawionej na straznikow
    free(l.poczatek);
    free(l.koniec);
  }
  return pozostali;
}