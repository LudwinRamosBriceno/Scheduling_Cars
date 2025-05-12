#include <stdio.h>
#include <stdlib.h>
#include "CEThread.h"

#define NUM_HILOS 3
#define ITERACIONES 20000000

#define LADO_IZQUIERDO 0
#define LADO_DERECHO   1

int contador_compartido1 = 0;
int contador_compartido2 = 0;
CEmutex_t mutex1;
CEmutex_t mutex2;

int incrementar1() {
    for (int i = 0; i < ITERACIONES; i++) {
        CEmutex_lock(&mutex1);  // ¡COMENTAR PARA VER EL FALLO!
        contador_compartido1++;
        //printf("EL contador es: %d, el indice es: %d \n", contador_compartido, i);
        CEmutex_unlock(&mutex1); // ¡COMENTAR PARA VER EL FALLO!
    }
    return 0;
}

int incrementar2() {
    for (int i = 0; i < ITERACIONES; i++) {
        //CEmutex_lock(&mutex2);  // ¡COMENTAR PARA VER EL FALLO!
        contador_compartido2++;
        //printf("EL contador es: %d, el indice es: %d \n", contador_compartido, i);
        //CEmutex_unlock(&mutex2); // ¡COMENTAR PARA VER EL FALLO!
    }
    return 0;
}

int main() {
    CEthread_t* hilos[NUM_HILOS];  // Array de punteros
    
    CEmutex_init(&mutex1);
    //CEmutex_init(&mutex2);
    
    // Crear hilos
    /* for (int i = 0; i < NUM_HILOS; i++) {
        CEthread_create(&hilos[i], NULL, incrementar, NULL);
    } */

    set_lado_en_cambio_contexto_RR(LADO_DERECHO);  // Para el algoritmo RR, se necesita indicar la cola con la que se trabajará, pues tiene cambio de contexto
    CEthread_create(&hilos[0], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_DERECHO);
    CEthread_create(&hilos[1], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_DERECHO);
    CEthread_create(&hilos[2], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_DERECHO);
    //CEthread_create(&hilos[3], NULL, incrementar2, NULL, PRIORITY_NORMAL, LADO_IZQUIERDO);
    //CEthread_create(&hilos[4], NULL, incrementar2, NULL, PRIORITY_NORMAL, LADO_IZQUIERDO);
    
    // Esperar hilos
    for (int i = 0; i < NUM_HILOS; i++) {
        CEthread_join(hilos[i]);
    }
    
    // Resultados
    printf("Valor FINAL: %d (debería ser %d)\n", 
           contador_compartido1, NUM_HILOS * ITERACIONES);
    printf("Valor FINAL: %d (debería ser %d)\n", 
           contador_compartido2, NUM_HILOS * ITERACIONES);
    
    CEmutex_destroy(&mutex1);
    //CEmutex_destroy(&mutex2);
    return 0;
}


// Compilar código:
// gcc CEThread_utils.c Calendarizador.c CEThread.c prueba_mutex.c -o prueba_mutex
