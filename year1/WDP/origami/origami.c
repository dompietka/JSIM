#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>

const double eps = 0.000001;

typedef struct punkt {
  double x;
  double y;
}
punkt;

punkt nowy_punkt(double x, double y) {
  punkt p;
  p.x = x;
  p.y = y;
  return p;
}

typedef struct prosta {
  double a;
  double b;
  double c;
}
prosta;

prosta nowa_prosta(punkt p1, punkt p2) {
  prosta l;
  l.a = p2.y - p1.y;
  l.b = p1.x - p2.x;
  l.c = p2.x * p1.y - p2.y * p1.x;
  return l;
}

int polozenie_wzgledem_prostej(punkt p, prosta l) {
  double x = l.a * p.x + l.b * p.y + l.c;
  if (x >= 0 && x < eps) return 0; //gdy na prostej
  if (x < 0) return 1; // gdy po lewej od prostej
  if (x > eps) return -1; // gdy po prawej od prostej

  return 0;
}

punkt symetria(punkt p, prosta l) {
  punkt q;
  q.x = p.x - (2 * l.a * (l.a * p.x + l.b * p.y + l.c)) / (l.a * l.a + l.b * l.b);
  q.y = p.y - (2 * l.b * (l.a * p.x + l.b * p.y + l.c)) / (l.a * l.a + l.b * l.b);
  return q;
}

typedef struct element {
  punkt p;
  struct element * poprzedni;
  struct element * kolejny;
}
element;

typedef struct lista {
  element * poczatek;
  element * koniec;
}
lista;

void dodaj(lista * q, punkt p) {
  element * x = malloc(sizeof(element));
  x -> p = p;
  x -> kolejny = NULL;

  if (q -> poczatek == NULL) {
    x -> poprzedni = NULL;
    q -> poczatek = x;
  } else {
    x -> poprzedni = q -> koniec;
    q -> koniec -> kolejny = x;
  }

  q -> koniec = x;

  return;
}

void usun(lista * q, element * x) {
  if (x == NULL) return;

  if (x == q -> poczatek)
    q -> poczatek = x -> kolejny;
  else
    x -> poprzedni -> kolejny = x -> kolejny;

  if (x == q -> koniec)
    q -> koniec = x -> poprzedni;
  else
    x -> kolejny -> poprzedni = x -> poprzedni;

  free(x);
  return;
}

typedef struct element_prosta {
  prosta l;
  struct element_prosta * poprzedni;
  struct element_prosta * kolejny;
}
element_prosta;

typedef struct lista_prostych {
  element_prosta * poczatek;
  element_prosta * koniec;
}
lista_prostych;

void dodaj_prosta(lista_prostych * q, prosta l) {
  element_prosta * x = malloc(sizeof(element_prosta));
  x -> l = l;
  x -> kolejny = NULL;

  if (q -> poczatek == NULL) {
    x -> poprzedni = NULL;
    q -> poczatek = x;
  } else {
    x -> poprzedni = q -> koniec;
    q -> koniec -> kolejny = x;
  }

  q -> koniec = x;

  return;
}

typedef struct kartka {
  bool ksztalt;
  punkt p1;
  punkt p2;
  punkt srodek;
  double promien;
  lista_prostych zgiecia;
}
kartka;

kartka nowa_kartka(bool k, punkt p1, punkt p2, punkt s, double r) {
  kartka a;
  a.ksztalt = k; //0 gdy koło, 1 gdy prostokąt
  a.p1 = p1;
  a.p2 = p2;
  a.srodek = s;
  a.promien = r;
  a.zgiecia.koniec = NULL;
  a.zgiecia.poczatek = NULL;
  return a;
}

bool nalezy(kartka * k, punkt p) {
  //koło
  if (k -> ksztalt == 0) {
    if ((p.x - k -> srodek.x) * (p.x - k -> srodek.x) + (p.y - k -> srodek.y) * (p.y - k -> srodek.y) <= (k -> promien + eps) * (k -> promien + eps) )
      return true;
    else
      return false;
  }

  //prostokąt
  if (((p.x <= k -> p1.x + eps && p.x + eps >= k -> p2.x) || (p.x + eps >= k -> p1.x && p.x <= k -> p2.x  + eps)) && ((p.y <= k -> p1.y  + eps && p.y + eps >= k -> p2.y) || (p.y  + eps >= k -> p1.y && p.y <= k -> p2.y + eps)))
    return true;
  else
    return false;
}

int ile_warstw(kartka * k, punkt p) {
  int wynik = 0;

  lista punkty; //punkty które mogą znajdować się na kartce przed zgięciami
  punkty.koniec = NULL;
  punkty.poczatek = NULL;

  dodaj( & punkty, p);

  lista_prostych linie = k -> zgiecia;
  element_prosta * z = linie.koniec;
  element * pinezka = punkty.koniec;
  int polozenie;

  while (z != NULL) { //rozważamy wszystkie proste wzdłuż których była zginana ta kartka
    pinezka = punkty.koniec;
    while (pinezka != NULL) {
      polozenie = polozenie_wzgledem_prostej(pinezka -> p, z -> l);
      // jesli polozenie = 0 nic sie nie dzieje, już jest na liście
      if (polozenie > 0) { //jeśli po lewej, to mogł być tam wcześniej lub zostać nałożony po zgięciu
        dodaj( & punkty, symetria(pinezka -> p, z -> l));
      }
      if (polozenie < 0) { // jeśli po prawej to na pewno nie ma tam kartki
        usun( & punkty, pinezka);
      }
      pinezka = pinezka -> poprzedni;

    }
    z = z -> poprzedni;
  }

  pinezka = punkty.poczatek;
  while (pinezka != NULL) {
    if (nalezy(k, pinezka -> p))
      wynik++;
    pinezka = pinezka -> kolejny;
  }

  return wynik;

}

int main() {

  int n, q, k;
  double x1, x2, y1, y2, r;

  scanf("%d %d", & n, & q);
  kartka * kartki = malloc((long unsigned int)(n + 1) * sizeof(kartka));
  
  char znak;
  for (int i = 1; i <= n; i++) {
    scanf(" %c", & znak);
    if (znak == 'P') {
      scanf("%lf %lf %lf %lf", & x1, & y1, & x2, & y2);
      kartki[i] = nowa_kartka(1, nowy_punkt(x1, y1), nowy_punkt(x2, y2), nowy_punkt(0, 0), 0);
    }
    if (znak == 'K') {
      scanf("%lf %lf %lf", & x1, & y1, & r);
      kartki[i] = nowa_kartka(0, nowy_punkt(0, 0), nowy_punkt(0, 0), nowy_punkt(x1, y1), r);
    }
    if (znak == 'Z') {
      scanf("%d %lf %lf %lf %lf", & k, & x1, & y1, & x2, & y2);
      kartki[i] = kartki[k];
      dodaj_prosta( & kartki[i].zgiecia, nowa_prosta(nowy_punkt(x1, y1), nowy_punkt(x2, y2)));
    }
  }

  for (int i = 0; i < q; i++) {
    scanf("%d %lf %lf", & k, & x1, & y1);
    printf("%d\n", ile_warstw( & kartki[k], nowy_punkt(x1, y1)));
  }

}
