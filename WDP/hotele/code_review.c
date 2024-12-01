#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

int max(int a, int b){
    if (a >= b) return a;
    return b;
}

int min(int a, int b){
    if (a <= b) return a;
    return b;
}

// Funkcja sprawdzajaca czy motele A, B, C są roznych sieci
bool rozne_sieci(int *siec, unsigned int A, unsigned int B, unsigned int C){
    if(siec[A] == siec[B]) return false;
    if(siec[B] == siec[C]) return false;
    if(siec[C] == siec[A]) return false;

    return true;
}

int maksimum_z_najblizszych(unsigned int n, int *siec, int *odleglosc){
    int wynik_min = INT_MAX;
    bool istnieje_trojka = false;
    unsigned int A, C, i = 1;

    while(i < n){
        // Najpierw szukamy dwoch moteli roznych sieci
        if(siec[i - 1] == siec[i]){
            i++;
            continue;
        }

        // Szukamy najblizszego motelu innej sieci niz i oraz i-1
        C = i + 1;
        while(C < n && !rozne_sieci(siec, i - 1, i, C)){
            C++;
        }

        if(C == n) break;

        // Szukamy najkrotszego podprzedzialu, na ktorym sa motele kazdej z sieci i, (i-1), C
        // Jest on postaci (A, B, B, ..., B, C)
        A = C - 2;
        while(siec[A] == siec[C - 1]) A--;
        
        for(unsigned int B = A + 1; B < C; B++){
            wynik_min = min(wynik_min, max(odleglosc[B] - odleglosc[A], odleglosc[C] - odleglosc[B]));
            istnieje_trojka = true;
        }

        // Idziemy raz do przodu (iter C), nastepnie cofamy sie iter A (nie dalej niz do i-1), +liczenie wyniku,
        // wiec kazdy przedzial [i-1, C] przechodzimy max 3 razy
        i = C;
    }

    if(!istnieje_trojka) return 0;
    return wynik_min;
}

// Generujemy trzech kandydatow na skrajnie pierwszy motel i trzech na skrajnie ostatni
void generator_skrajnych_kandydatow(unsigned int n, int *siec, unsigned int *A, unsigned int *C){
    A[0] = 0;
    A[1] = 0;
    A[2] = 0;
    unsigned int j = 1;
    for(unsigned int i = 1; i < n; i++){
        if(siec[i] != siec[A[0]] && siec[i] != siec[A[1]]){
            A[j++] = i;
            if(j >= 3) break;
        }
    }

    C[0] = n - 1;
    C[1] = n - 1;
    C[2] = n - 1;
    j = 1;
    for(unsigned int i = n - 2; (int) i >= 0; i--){
        if(siec[i] != siec[C[0]] && siec[i] != siec[C[1]]){
            C[j++] = i;
            if(j >= 3) break;
        }
    }
}

// Funkcja do obliczania wynikow dla funkcji 'minimum_z_najdalszych'
int wynik_max_dla_kandydatow(int *siec, int *odleglosc, unsigned int A, unsigned int C){
    int wynik_max = -1;
    for(unsigned int B = A + 1; B < C; B++){
        if(rozne_sieci(siec, A, B, C)){
            wynik_max = max(wynik_max, min(odleglosc[B] - odleglosc[A], odleglosc[C] - odleglosc[B]));
        }
    }

    return wynik_max;
}

int minimum_z_najdalszych(unsigned int n, int *siec, int *odleglosc){    
    unsigned int *A = malloc(3 * sizeof(int));
    unsigned int *C = malloc(3 * sizeof(int));
    generator_skrajnych_kandydatow(n, siec, A, C);

    // A[2] == 0, wtedy i tylko wtedy, gdy wszystkie motele sa co najwyzej dwoch roznych sieci
    if(A[2] == 0) return 0;

    int wynik_max = -1;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(siec[A[i]] != siec[C[j]]){
                wynik_max = max(wynik_max, wynik_max_dla_kandydatow(siec, odleglosc, A[i], C[j]));
            }
        }
    }

    free(A);
    free(C);

    return wynik_max;
}

int main(){
    unsigned int n;
    scanf("%u", &n);

    int *siec = malloc(n * sizeof(int));
    int *odleglosc = malloc(n * sizeof(int));

    for(unsigned int i = 0; i < n; i++){
        scanf("%d %d", &siec[i], &odleglosc[i]);
    }

    int wynik_min = maksimum_z_najblizszych(n, siec, odleglosc);
    int wynik_max = minimum_z_najdalszych(n, siec, odleglosc);
    
    printf("%d %d\n", wynik_min, wynik_max);

    free(siec);
    free(odleglosc);

    return 0;
}