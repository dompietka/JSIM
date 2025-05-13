#include <bits/stdc++.h>
using namespace std;


int range=1000;
int n;
int main(){

    cin >> n;
    cout << n << " "; 
    srand(2137);
    
    for (int i =0;i<n;i++)
    {
        for (int j=0;j<n;j++)
        {
            cout << rand()%range << " ";
        }
    }
}