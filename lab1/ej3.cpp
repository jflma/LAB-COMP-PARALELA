#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <cassert>

using namespace std;
using namespace std::chrono;

// Multiplicacion por bloques
void multiplyBlocked(const vector<vector<double>>& A,
                     const vector<vector<double>>& B,
                     vector<vector<double>>& C,
                     int N, int Bsize) {
   
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            C[i][j] = 0.0;

    for (int ii = 0; ii < N; ii += Bsize) {
        int iimax = min(ii + Bsize, N);
        for (int jj = 0; jj < N; jj += Bsize) {
            int jjmax = min(jj + Bsize, N);
            for (int kk = 0; kk < N; kk += Bsize) {
                int kkmax = min(kk + Bsize, N);

                // bloque interior
                for (int i = ii; i < iimax; ++i) {
                    for (int j = jj; j < jjmax; ++j) {
                        double sum = C[i][j];
                        for (int k = kk; k < kkmax; ++k) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] = sum;
                    }
                }

            }
        }
    }
}

void multiplyClassic(const vector<vector<double>>& A,
                     const vector<vector<double>>& B,
                     vector<vector<double>>& C,
                     int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

bool matricesEqual(const vector<vector<double>>& X,
                   const vector<vector<double>>& Y,
                   int N, double tol = 1e-6) {
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (fabs(X[i][j] - Y[i][j]) > tol)
                return false;
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << fixed << setprecision(3);

    // Tamanos a evaluar
    vector<int> sizes = {20, 40, 60, 80};
    // Tamaños de bloque a probar
    vector<int> blocks = {8, 16, 32, 64};

    cout << "N\tB\tTiempo(ms)\tVerificacion\n";
    for (int N : sizes) {
        vector<vector<double>> A(N, vector<double>(N));
        vector<vector<double>> B(N, vector<double>(N));
        vector<vector<double>> C(N, vector<double>(N, 0.0));

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                A[i][j] = (double)((i + j) % 10 + 1); 
                B[i][j] = (double)(((i * 7 + j * 13) % 11) + 1);
            }
        }

        vector<vector<double>> Cref;
        bool use_ref = (N <= 400); 
        if (use_ref) {
            Cref.assign(N, vector<double>(N, 0.0));
            multiplyClassic(A, B, Cref, N);
        }

        for (int Bsize : blocks) {
            if (Bsize > N) continue;

            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j)
                    C[i][j] = 0.0;

            auto start = high_resolution_clock::now();

            multiplyBlocked(A, B, C, N, Bsize);

            auto stop = high_resolution_clock::now();
            auto dur = duration_cast<milliseconds>(stop - start).count();

            bool ok = true;
            if (use_ref) ok = matricesEqual(C, Cref, N);

            cout << N << "\t" << Bsize << "\t" << dur << "\t\t" << (ok ? "OK" : "ERROR") << "\n";
        }
    }

    return 0;
}
