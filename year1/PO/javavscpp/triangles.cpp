#include <iostream>
#include <chrono>
using namespace std;

long long n;
int main(){
    auto start = std::chrono::high_resolution_clock::now();
    cin >> n;
    long long a[n][n];
    for (long long i=0;i<n*n;i++){
        cin >> a[i/n][i%n];
    }

    long long b[n][n];
    long long c[n][n];

    for (long long i=0;i<n;i++){
        for (long long j =0;j<n;j++)
        {
            b[i][j]=a[i][j];
            c[i][j]=0;
        }
        
    }

    for (long long i=0;i<n;i++){
        for (long long j=0;j<n;j++){
            for (long long k=0;k<n;k++){
                c[i][j]+=a[i][k]*b[k][j];
            }
        }
    }

    /*for (long long i=0;i<n;i++){
        for (long long j =0;j<n;j++)
        {
            cout << c[i][j] << " ";
        }
        cout << "\n";
        
    }*/

    for (long long i=0;i<n;i++){
        for (long long j =0;j<n;j++)
        a[i][j]=0;
    }

    for (long long i=0;i<n;i++){
        for (long long j=0;j<n;j++){
            for (long long k=0;k<n;k++){
                a[i][j]+=c[i][k]*b[k][j];
            }
        }
    }
    /*for (long long i=0;i<n;i++){
        for (long long j =0;j<n;j++)
        {
            cout << a[i][j] << " "; 
        }
        cout << "\n";
        
    }*/

    long long wynik=0;
    for (long long i=0;i<n;i++)
    wynik+=a[i][i];
    
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << "czas w ms: " << duration.count();


}