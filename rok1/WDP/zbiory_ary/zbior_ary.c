#include "zbior_ary.h"

#include <stdio.h>

#include <stdlib.h>

#include <stdbool.h>

#include <limits.h>

/* tak wygląda struct do zbiorów
struct element{
  int poczatek;
  int koniec;
  int modulo;
};

typedef struct zbior_ary {
  unsigned ary; // ile podzbiorów
  int q; //roznica podzbioru
  struct element *podzbiory; //posortowna najpierw względem modulo a następnie względem elementów tablica spojnych podzbiorów
} zbior_ary;
*/

int max(int a, int b) {
    if (a >= b) return a;
    else return b;
}

int min(int a, int b) {
    if (a <= b) return a;
    else return b;
}

int roznica_ciagu = 0;

zbior_ary ciag_arytmetyczny(int a, int q, int b) {
    zbior_ary A;

    if (roznica_ciagu == 0) roznica_ciagu = q; //ustawienie q jako zmiennej globalnej

    A.ary = 1;
    A.q = q;
    A.podzbiory = (struct element * ) malloc(sizeof(struct element) * A.ary);

    A.podzbiory[0].poczatek = a;
    A.podzbiory[0].koniec = b;
    A.podzbiory[0].modulo = (a % q + q) % q;

    return A;
}

zbior_ary singleton(int a) {
    return ciag_arytmetyczny(a, roznica_ciagu, a);
}

zbior_ary suma(zbior_ary A, zbior_ary B) {
    if (A.ary == 0) return B;
    if (B.ary == 0) return A;

    zbior_ary C;
    C.ary = A.ary + B.ary;
    C.q = roznica_ciagu;
    C.podzbiory = (struct element * ) malloc(C.ary * sizeof(struct element));

    int i = 0;
    int j = 0;
    int ary_c = 1; //ile faktycznych podciągów C oraz jednocześnie indeks kolejnego który będziemy dodawać

    //pierwszy element C czyli najmniejszy najpierw po modulo potem po początku
    if (A.podzbiory[0].modulo == B.podzbiory[0].modulo) {
        if (A.podzbiory[0].poczatek <= B.podzbiory[0].poczatek) {
            C.podzbiory[0] = A.podzbiory[0];
            i++;
        } else {
            C.podzbiory[0] = B.podzbiory[0];
            j++;
        }
    } else {
        if (A.podzbiory[0].modulo < B.podzbiory[0].modulo) {
            C.podzbiory[0] = A.podzbiory[0];
            i++;
        } else {
            C.podzbiory[0] = B.podzbiory[0];
            j++;
        }

    }

    while (i < (int) A.ary && j < (int) B.ary) {
        //dopisuje i-ty podzbior A do C, ponieważ jest mniejszy od j-tego podzbioru B
        if (A.podzbiory[i].modulo < B.podzbiory[j].modulo || (A.podzbiory[i].modulo == B.podzbiory[j].modulo && A.podzbiory[i].poczatek <= B.podzbiory[j].poczatek)) {
            if (A.podzbiory[i].modulo > C.podzbiory[ary_c - 1].modulo) {
                C.podzbiory[ary_c] = A.podzbiory[i];
                i++;
                ary_c++;
            } else {
                //gdy tworzy spojny podzbior z poprzednim elementem C
                if (A.podzbiory[i].poczatek <= C.podzbiory[ary_c - 1].koniec + C.q) {
                    C.podzbiory[ary_c - 1].koniec = max(C.podzbiory[ary_c - 1].koniec, A.podzbiory[i].koniec);
                    i++;
                } else {
                    C.podzbiory[ary_c] = A.podzbiory[i];
                    i++;
                    ary_c++;
                }
            }
        }
        //analogicznie dla j-tego elementu B gdy jest mniejszy
        else {
            if (B.podzbiory[j].modulo > C.podzbiory[ary_c - 1].modulo) {
                C.podzbiory[ary_c] = B.podzbiory[j];
                j++;
                ary_c++;
            } else {
                if (B.podzbiory[j].poczatek <= C.podzbiory[ary_c - 1].koniec + C.q) {
                    C.podzbiory[ary_c - 1].koniec = max(C.podzbiory[ary_c - 1].koniec, B.podzbiory[j].koniec);
                    j++;
                } else {
                    C.podzbiory[ary_c] = B.podzbiory[j];
                    j++;
                    ary_c++;
                }
            }
        }
    }

    //jesli dodalismy caly zbior B ale pozostalu niedodane fragmenty z A
    while (i < (int) A.ary) {
        //sprawdzenie czy nie jest spojny z poprzednim
        if (A.podzbiory[i].modulo == C.podzbiory[ary_c - 1].modulo && A.podzbiory[i].poczatek <= C.podzbiory[ary_c - 1].koniec + C.q) {
            C.podzbiory[ary_c - 1].koniec = max(C.podzbiory[ary_c - 1].koniec, A.podzbiory[i].koniec);
        } else {
            C.podzbiory[ary_c] = A.podzbiory[i];
            ary_c++;
        }
        i++;
    }

    //jesli dodalismy caly zbior A ale pozostalu niedodane fragmenty z B, analogicznie
    while (j < (int) B.ary) {
        if (B.podzbiory[j].modulo == C.podzbiory[ary_c - 1].modulo && B.podzbiory[j].poczatek <= C.podzbiory[ary_c - 1].koniec + C.q) {
            C.podzbiory[ary_c - 1].koniec = max(C.podzbiory[ary_c - 1].koniec, B.podzbiory[j].koniec);
        } else {
            C.podzbiory[ary_c] = B.podzbiory[j];
            ary_c++;
        }
        j++;
    }

    //przepisanie do nowego zbioru, tak aby zajmowało dokadnie tyle pamięci ile wymaga, bez pustych elementów
    zbior_ary D;
    D.ary = (unsigned) ary_c;
    D.q = roznica_ciagu;

    D.podzbiory = (struct element * ) malloc(sizeof(struct element) * D.ary);
    for (i = 0; i < (int) D.ary; i++) {
        D.podzbiory[i] = C.podzbiory[i];
    }
    free(C.podzbiory);
    return D;

}

zbior_ary iloczyn(zbior_ary A, zbior_ary B) {
    if (A.ary == 0) return A;
    if (B.ary == 0) return B;
    zbior_ary C;
    C.ary = (unsigned) max((int) A.ary, (int) B.ary);
    C.q = roznica_ciagu;

    C.podzbiory = (struct element * ) malloc(sizeof(struct element) * C.ary);

    int ary_c = 0;
    int j = 0;
    int i = 0;

    while (i < (int) A.ary && j < (int) B.ary) {
        //porownujemy tylko elementy z takim samym modulo
        if (A.podzbiory[i].modulo < B.podzbiory[j].modulo) {
            i++;
        } else {
            if (A.podzbiory[i].modulo > B.podzbiory[j].modulo) {
                j++;
            } else {
                while (i < (int) A.ary && j < (int) B.ary && A.podzbiory[i].modulo == B.podzbiory[j].modulo) {
                    //czy maja niepuste przeciecie, jesli tak dopisanie go do C
                    if ((A.podzbiory[i].poczatek <= B.podzbiory[j].poczatek && A.podzbiory[i].koniec >= B.podzbiory[j].poczatek) ||
                        (B.podzbiory[j].poczatek <= A.podzbiory[i].poczatek && B.podzbiory[j].koniec >= A.podzbiory[i].poczatek)) {
                        C.podzbiory[ary_c].modulo = A.podzbiory[i].modulo;
                        C.podzbiory[ary_c].poczatek = max(A.podzbiory[i].poczatek, B.podzbiory[j].poczatek);
                        C.podzbiory[ary_c].koniec = min(A.podzbiory[i].koniec, B.podzbiory[j].koniec);
                        ary_c++;
                    }
                    //ktora pare rozwazamy jako kolejna, (i,j+1) czy (i+1,j)
                    if (i + 1 >= (int) A.ary || (A.podzbiory[i + 1].modulo > A.podzbiory[i].modulo) || (j + 1 < (int) B.ary && B.podzbiory[j + 1].poczatek <= A.podzbiory[i + 1].poczatek))
                        j++;
                    else
                        i++;
                }
            }
        }
    }

    //przepisanie do przeskalowanej struktury
    zbior_ary D;
    D.ary = (unsigned) ary_c;
    D.q = roznica_ciagu;

    D.podzbiory = (struct element * ) malloc(sizeof(struct element) * D.ary);
    for (i = 0; i < (int) D.ary; i++) {
        D.podzbiory[i] = C.podzbiory[i];
    }
    free(C.podzbiory);
    return D;

}

zbior_ary roznica(zbior_ary A, zbior_ary B) {
    if (A.ary == 0 || B.ary == 0) return A;
    zbior_ary C;
    C.q = roznica_ciagu;
    C.ary = A.ary + B.ary + (unsigned) C.q;

    C.podzbiory = (struct element * ) malloc(sizeof(struct element) * C.ary);
    int j = 0;
    int ary_c = 0;
    long long koniec = 0;
    for (int i = 0; i < (int) A.ary; i++) {
        //jesli odejmowany zbior sie skonczyl
        if (j >= (int) B.ary) {
            C.podzbiory[ary_c] = A.podzbiory[i];
            ary_c++;
        } else {
            if (A.podzbiory[i].modulo < B.podzbiory[j].modulo) {
                C.podzbiory[ary_c] = A.podzbiory[i];
                ary_c++;
            } else {
                while (j < (int) B.ary && A.podzbiory[i].modulo > B.podzbiory[j].modulo) {
                    j++;
                }
                while (j < (int) B.ary && A.podzbiory[i].poczatek > B.podzbiory[j].koniec) {
                    j++;;
                }
                if (j < (int) B.ary && B.podzbiory[j].modulo == A.podzbiory[i].modulo) {
                    //jesli odejmowany fragment zaczyna sie przed podzbiorem A, ale maja elementy wspolne
                    if (A.podzbiory[i].poczatek >= B.podzbiory[j].poczatek) {
                        koniec = (long long) B.podzbiory[j].koniec + B.q;

                        if (koniec <= (long long) A.podzbiory[i].koniec) j++;
                    } else {
                        koniec = (long long) A.podzbiory[i].poczatek;
                    }

                    //odejmujemy wszytskie podciagi b ktore zawieraja sie w danym podciagu A (dodajemy fragment ktory konczy sie tuz przed odjetym framentem)
                    while (j < (int) B.ary && A.podzbiory[i].modulo == B.podzbiory[j].modulo && koniec <= (long long) A.podzbiory[i].koniec && A.podzbiory[i].koniec >= B.podzbiory[j].poczatek) {
                        C.podzbiory[ary_c].modulo = A.podzbiory[i].modulo;
                        C.podzbiory[ary_c].poczatek = (int) koniec;
                        C.podzbiory[ary_c].koniec = B.podzbiory[j].poczatek - C.q;

                        koniec = (long long) B.podzbiory[j].koniec + B.q;

                        ary_c++;
                        if (koniec <= (long long) A.podzbiory[i].koniec) j++;
                    }

                    //ostatni fragment miedzy ostatnim odjętym a końcem danego podciagu A
                    if (koniec <= (long long) A.podzbiory[i].koniec) {
                        C.podzbiory[ary_c].modulo = A.podzbiory[i].modulo;
                        C.podzbiory[ary_c].poczatek = (int) koniec;
                        C.podzbiory[ary_c].koniec = A.podzbiory[i].koniec;
                        ary_c++;
                    }
                } else {
                    //jesli po przeitoreowaniu po j w poczatkowych while'ach zbior odejmowany sie skonczyl
                    C.podzbiory[ary_c] = A.podzbiory[i];
                    ary_c++;
                }
            }
        }
    }

    //przepisanie
    zbior_ary D;
    D.ary = (unsigned) ary_c;
    D.q = roznica_ciagu;

    D.podzbiory = (struct element * ) malloc(sizeof(struct element) * D.ary);
    for (int i = 0; i < (int) D.ary; i++) {
        D.podzbiory[i] = C.podzbiory[i];
    }
    free(C.podzbiory);
    return D;

}

bool nalezy(zbior_ary A, int b) {
    int l = 0;
    int p = (int) A.ary;
    int s;
    int b_modulo = (b % A.q + A.q) % A.q;
    if ((int) A.ary == 0) return 0;

    //szukamy pierwszego podzbioru z tym modulo
    while (l < p) {
        s = (l + p) / 2;
        if (A.podzbiory[s].modulo >= b_modulo) p = s;
        else l = s + 1;
    }

    int pierwszy;
    if (A.podzbiory[p].modulo == b_modulo)
        pierwszy = p;
    else
        pierwszy = p + 1;

    if (pierwszy >= (int) A.ary || A.podzbiory[pierwszy].modulo != b_modulo) return 0;

    //szukamy ostatniego podzbioru z tym modulo
    l = 0;
    p = (int) A.ary;
    while (l < p) {
        s = (l + p) / 2;
        if (A.podzbiory[s].modulo <= b_modulo) l = s + 1;
        else p = s;
    }

    int ostatni = p - 1;

    //jesli jest tylko jeden przedzial
    if (pierwszy == ostatni) {
        if (A.podzbiory[pierwszy].poczatek <= b && A.podzbiory[pierwszy].koniec >= b)
            return 1;
        else
            return 0;
    }
    //pierwszy zaczynajacy sie za b
    while (pierwszy < ostatni) {
        s = (pierwszy + ostatni) / 2;
        if (A.podzbiory[s].poczatek <= b) pierwszy = s + 1;
        else ostatni = s;

    }

    //czy b sie zawiera w ostatnim zaczynajacym sie przed b (lub jesli ostatni to ten z indeksem 0)
    if ((A.podzbiory[ostatni].poczatek <= b && A.podzbiory[ostatni].koniec >= b) ||
        (ostatni > 0 && (A.podzbiory[ostatni - 1].poczatek <= b && A.podzbiory[ostatni - 1].koniec >= b)))
        return 1;
    else
        return 0;
}

unsigned moc(zbior_ary A) {
    unsigned long int w = 0;

    for (int i = 0; i < (int) A.ary; i++) {
        w += (unsigned long int)(((long long) A.podzbiory[i].koniec - (long long) A.podzbiory[i].poczatek) / A.q) + 1;
    }
    return (unsigned) w;
}

unsigned ary(zbior_ary A) {
    return A.ary;
}