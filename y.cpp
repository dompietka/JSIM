#include "zbior_ary.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* tak wygląda struct do zbiorów
typedef struct zbior_ary {
  int ary; // ile podzbiorów
  int q; //różnica zbioru
  int *zbior; //wskaźnik na tablice w której są te tablice
} zbior_ary;
*/

void usun_zbior(zbior_ary *A){ //uzywac jako usub_zbior( &A ) (chyba) (mam nadzieje ze zadziala) (prosze)
    free(A->zbior);
    free(A);
}

zbior_ary ciag_arytmetyczny(int a, int q, int b){
    zbior_ary A;
    A.ary=1;
    A.q=q;
    A.zbior = (int*)malloc(2*A.ary*sizeof(int));
    A.zbior[0]=a;
    A.zbior[1]=b;

    return A;
}

zbior_ary singleton(int a) {
    zbior_ary A;
    A.ary=1;
    A.q=1;
    A.zbior=(int*)malloc(2*A.ary*sizeof(int));
    A.zbior[0]=a;
    A.zbior[1]=a;

    return A;
}

zbior_ary suma(zbior_ary A, zbior_ary B){
    zbior_ary C;
    C.ary=A.ary+B.ary;
    C.zbior=(int*)malloc(2*C.ary*sizeof(int));
    int i=0; 
    int j=0;
    int ary_c=0;
    if (A.zbior[0] < B.zbior[0])
    {
        ary_c++;
        i++;
        C.zbior[0]=A.zbior[0];
        C.zbior[1]=A.zbior[1];
    }
    while ( i < A.ary && j < B.ary){
        
    }
}

zbior_ary iloczyn(zbior_ary A, zbior_ary B);


zbior_ary roznica(zbior_ary A, zbior_ary B);

bool nalezy(zbior_ary A, int b){
    int mod = b%A.q;
    int l=0;
    int p=A.ary;
    while (l!=p)
    {
        int s = (l+p)/2;
        if (A.zbior[2*s] <= b)
            l=s;
        else
            p=s-1;
    }

    printf("ostatni zaczynajacy się przed ")
}

unsigned moc(zbior_ary A){
    int w=0;
    for (int i=0;i<2*A.ary;i+=2)
    {
        w += (A.zbior[i+1]-A.zbior[i])/A.q + 1;
    }
    return w;
}

unsigned ary(zbior_ary A){
    return A.ary;
}



int main(){

printf("%d", moc(ciag_arytmetyczny(1, 2, 7)));
}