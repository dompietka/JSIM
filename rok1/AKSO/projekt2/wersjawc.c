#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>

uint64_t shld(uint64_t A, uint64_t B, unsigned n) {
//    if (n == 0)
//        printf ("%lx %lx %lx\n", A, B, (A << n) | (B >> (64 - n)));
    if (n == 0)
        return A;
    return (A << n) | (B >> (64 - n));
}



void nsqrt(uint64_t*Q, uint64_t *X, unsigned n){

    for (int i=0;i<n/64;i++){
        Q[i]=0;
    }
    for (int j=1;j<n;j++){ 
//        printf("i = %d, X = %lx|%lx|%lx|%lx,  Q = %lx|%lx|%lx\n", n-j, X[3], X[2], X[1], X[0], Q[2], Q[1], Q[0]);
        //zapalam ten bit w q
        Q[(n-j-1)/64]|=(1ULL << (n-j-1)%64);


        bool x=1;
        uint64_t A, B;

        for (int i=n/32-1;i>=0;i--){

            uint64_t a = i-((n-j+1)/64);

            // A = 0
            if (a>=0 && a < (n/64)) // if(a < 0): goto 37  if(a >= n/64): goto 37
                A = Q[a]; //
            else 
                A=0;

            if (a>=1 && a <= (n/64)) //if (a<1) lub a > n/64
                B = Q[a-1];
            else 
                B=0;

            uint64_t t=shld(A,B,(n-j+1+64)%64);

//            if (n-j == 63)
//                printf("i = %d, T = %lx  X = %lx\n", i, t, X[i]);

            if (t>X[i]){
                x=0;
                break;
            }
            if (t<X[i])
                break;

            //if > set flaga na 0
            // if rowne continue
            
        }
        

        

        if (x==1)
        {

            //printf("zapalony bit %d\n", n-j);


        //odejmowanie t od r jesli bylo ok

            bool x1=0;
            bool x2=0;
             for (int i=0;i<n/32;i++){
                uint64_t a = i-((n-j+1)/64);

            if (a>=0 && a <= (n/64)-1)
                A = Q[a];
            else 
                A=0;

            if (a>=1 && a-1 <= (n/64)-1)
                B = Q[a-1];
            else 
                B=0;

            uint64_t t=shld(A,B,(n-j+1)%64);

            x2=0;
            if (t == UINT64_MAX && x1==1) x2=1;
            else {
                t+=x1;
                if (X[i]<t) x2=1;
                X[i]-=t;
            }
            x1=x2;


            
            }
            //printf("Q = %lu", Q[0]);
            Q[(n-j)/64]|=(1ULL << (n-j)%64);
            //printf(", Q = %lu\n", Q[0]);

        }
        Q[(n-j-1)/64] &= ~(1ULL << (n-j-1)%64);
        //printf(", Q = %lu\n", Q[0]);

        //printf("%lu\n", X[0]);
        //printf(" Q = %lu\n", Q[0]);
        }

        //ostatni fragment petli



        
        bool x=1;
        uint64_t A, B;
         for (int i=n/32-1;i>=0;i--){

            uint64_t a = i;

            if (a>=0 && a <= (n/64)-1) //if a > n 64-1 czyli a >= n64 
                A = Q[a];
            else 
                A=0;

            if (a>=1 && a <= (n/64)) //if a = 0 lub gdzie a > n64
                B = Q[a-1];
            else {
                if (a-1 == -1)
                    B = (1ULL<<63); //jesli a to 0
                else
                    B = 0;
   
            }

            uint64_t t=shld(A,B,(1)%64);

            if (t>X[i]){
                x=0;
                break;
            }
            if (t<X[i])
                break;

            
            
        }
        if (x==1) {
            Q[0]+=1; 
            //printf("here");
        }


}


