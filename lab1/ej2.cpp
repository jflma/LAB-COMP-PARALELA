#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

void multiplyClassic(const vector<vector<double>>& A,
                     const vector<vector<double>>& B,
                     vector<vector<double>>& C,
                     int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    // Diferentes tamaños de matrices a evaluar
    vector<int> sizes = {200, 400, 600, 800, 1000};

    cout << fixed << setprecision(6);

    for (int N : sizes) {
        vector<vector<double>> A(N, vector<double>(N, 1.0));
        vector<vector<double>> B(N, vector<double>(N, 1.0));
        vector<vector<double>> C(N, vector<double>(N, 0.0));

        auto start = high_resolution_clock::now();

        multiplyClassic(A, B, C, N);

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);

        cout << "Tamano: " << N << "x" << N
             << " --> Tiempo: " << duration.count() << " ms" << endl;
    }

    return 0;
}
