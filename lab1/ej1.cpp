#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

const int MAX = 1000; 

int main() {
    static double A[MAX][MAX];
    static double x[MAX];
    static double y[MAX];

    // Initialize A and x, assign y = 0
    for (int i = 0; i < MAX; i++) {
        x[i] = 1.0;
        for (int j = 0; j < MAX; j++) {
            A[i][j] = (i + j) % 10;
        }
    }

    for (int i = 0; i < MAX; i++) 
        y[i] = 0.0; 

    auto start1 = high_resolution_clock::now();

    //First pair of loops 
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            y[i] += A[i][j] * x[j];
        }
    }

    auto stop1 = high_resolution_clock::now();
    auto duration1 = duration_cast<microseconds>(stop1 - start1);
    cout << fixed << setprecision(6);
    cout << "Tiempo primer par de bucles: " << duration1.count() / 1000.0 << " ms" << endl;


    //Second pair of loops
    for (int i = 0; i < MAX; i++) 
        y[i] = 0.0;

    auto start2 = high_resolution_clock::now();

    for (int j = 0; j < MAX; j++) {
        for (int i = 0; i < MAX; i++) {
            y[i] += A[i][j] * x[j];
        }
    }

    auto stop2 = high_resolution_clock::now();
    auto duration2 = duration_cast<microseconds>(stop2 - start2);
    cout << "Tiempo segundo par de bucles: " << duration2.count() / 1000.0 << " ms" << endl;

    return 0;
}
