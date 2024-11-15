#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

int min(int a, int b)
{
    if (a <= b) return a;
    else return b;
}

int max(int a, int b)
{
    if (a >= b) return a;
    else return b;
}

int n;
int *type;
int *dist;
int *left;
int *right;

int closest(){ //maksimum z odległości jest minimalne
    int first, last, middle;
    int ans=INT_MAX;
    first=left[0];
    middle=left[1];
    last=left[2];

    while (last < n){ //gąsienica
        if (type[first]==type[last] || type[middle]==type[last]){
            last++;
            continue;
        }
            
        if (type[middle]==type[first]){
            middle++;
            continue;
        }

        ans=min(ans, max(dist[middle]-dist[first], dist[last]-dist[middle]));

        if (middle < last && type[middle]==type[middle+1]){ 
            middle++;
            continue;
        }

        first=middle; 
    }

    return ans;
}

bool diff_types(int k, int l, int m){ //czy sieci hoteli o nr k, l, m są różne
    if (type[k]==type[l] || type[l]==type[m] || type[k]==type[m])
    return 0;
    else 
    return 1;
}

int furthest(){ //minumum z odleglosci ma byc maksymalne
    int ans=0;


    int middle=1;
    while(middle < n){
        for (int l=0;l<3;l++){ // wszytskie kombinacje skrajnie lewy x skrajnie prawy
            for (int r=2;r>=0;r--){
                if (diff_types(left[l], middle, right[r])) 
                ans = max(ans, min(dist[middle]-dist[left[l]], dist[right[r]]-dist[middle])); 
                //jesli rozważany środek jest poza przedziałem (poczatek, koniec) lub koniec < początek wartość minimum z odległości będzie ujemna, czyli bez znaczenia
            }
        }
        middle++;
       
    }

    return ans;

}


int main(){
    assert(scanf("%d", &n));
    dist = (int *)malloc((long unsigned int)n*sizeof(int)); //odleglosc od poczatku dla hotelu z numerem i;
    type = (int *)malloc((long unsigned int)n*sizeof(int)); //siec hotelu z numerem i

    for (int i = 0; i < n; i++){
        assert(scanf("%d %d", &type[i], &dist[i]));
    }

    //numery trzech skrajnych z lewej i trzech skrajnych z prawej hoteli różnych sieci

    left = (int *)malloc(3*sizeof(int));
    right = (int *)malloc(3*sizeof(int));

    //lewe
    left[0]=0;
    int i=1;
    while(i < n && type[i]==type[left[0]])
        i++;
    if (i==n){ //wszystkie są tej samej sieci
        printf("0 0");
    }
    left[1]=i;
    while(i < n && (type[i]==type[left[0]] || type[i]==type[left[1]]))
        i++;
    if (i==n){ //są tylko 2 sieci
        printf("0 0");
    }
    left[2]=i;

    //prawe
    right[2]=n-1;
    i=n-2;
    while (type[i]==type[right[2]])
        i--;
    right[1]=i;
    while (type[i]==type[right[2]] || type[i]==type[right[1]])
        i--;
    right[0]=i;

    //printf("wszystko git \n");
    printf("%d %d\n", closest(), furthest());
}