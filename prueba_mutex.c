#include <stdio.h>
#include <stdlib.h>
#include "CEThread.h"

#define NUM_HILOS 5
#define ITERACIONES 100000

int contador_compartido = 0;
CEmutex_t mutex;

int incrementar(void* arg) {
    for (int i = 0; i < ITERACIONES; i++) {
        //CEmutex_lock(&mutex);  // ¡COMENTAR PARA VER EL FALLO!
        contador_compartido++;
        //CEmutex_unlock(&mutex); // ¡COMENTAR PARA VER EL FALLO!
    }
    return 0;
}

int main() {
    CEthread_t* hilos[NUM_HILOS];  // Array de punteros
    
    CEmutex_init(&mutex);
    
    // Crear hilos
    /* for (int i = 0; i < NUM_HILOS; i++) {
        CEthread_create(&hilos[i], NULL, incrementar, NULL);
    } */

    CEthread_create(&hilos[0], NULL, incrementar, NULL, PRIORITY_NORMAL);
    CEthread_create(&hilos[1], NULL, incrementar, NULL, PRIORITY_SPORTS);
    CEthread_create(&hilos[2], NULL, incrementar, NULL, PRIORITY_EMERGENCY);
    CEthread_create(&hilos[3], NULL, incrementar, NULL, PRIORITY_NORMAL);
    CEthread_create(&hilos[4], NULL, incrementar, NULL, PRIORITY_NORMAL);
    
    // Esperar hilos
    for (int i = 0; i < NUM_HILOS; i++) {
        CEthread_join(hilos[i]);
    }
    
    // Resultados
    printf("Valor FINAL: %d (debería ser %d)\n", 
           contador_compartido, NUM_HILOS * ITERACIONES);
    
    CEmutex_destroy(&mutex);
    return 0;
}


// Compilar código:
// gcc CEThread_utils.c Calendarizador.c CEThread.c prueba_mutex.c -o prueba_mutex
