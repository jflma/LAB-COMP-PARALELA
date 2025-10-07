#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    long n = 100000000; 
    double factor = 1.0;
    double sum = 0.0;
    double pi;
    clock_t inicio, fin;
    double tiempo_transcurrido;

    printf("Calculando pi con %ld iteraciones...\n", n);

    inicio = clock();

    for (long i = 0; i < n; i++, factor = -factor) {
        sum += factor / (2 * i + 1);
    }

    pi = 4.0 * sum;

    fin = clock();

    tiempo_transcurrido = (double)(fin - inicio) / CLOCKS_PER_SEC;

    printf("Aproximación de pi: %.15f\n", pi);
    printf("Tiempo de ejecución: %.3f segundos\n", tiempo_transcurrido);

    return 0;
}
