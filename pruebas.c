#define _GNU_SOURCE 
#include <time.h>
#include <stdio.h>

int main() {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);  // Tiempo real (no afectado por ajustes del sistema)
    
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;

        if (elapsed >= 2.0) {
            printf("¡Pasaron 2 segundos!\n");
            break;
        }
    }

    return 0;
}