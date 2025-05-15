#include <stdio.h>
#include <stdlib.h>
#include "CEThread.h"
#include "control_flujo.h"

#define NUM_HILOS 5
#define ITERACIONES 200000000

#define LADO_IZQUIERDO 0
#define LADO_DERECHO   1

int contador_compartido1 = 0;
int contador_compartido2 = 0;
CEmutex_t mutex1;
CEmutex_t mutex2;

int incrementar1() {
    for (int i = 0; i < ITERACIONES; i++) {
        //CEmutex_lock(&mutex1);  // ¡COMENTAR PARA VER EL FALLO!
        contador_compartido1++;
        //CEmutex_unlock(&mutex1); // ¡COMENTAR PARA VER EL FALLO!
    }
    return 0;
}

int incrementar2() {
    for (int i = 0; i < ITERACIONES; i++) {
        //CEmutex_lock(&mutex2);  // ¡COMENTAR PARA VER EL FALLO!
        contador_compartido2++;
        //CEmutex_unlock(&mutex2); // ¡COMENTAR PARA VER EL FALLO!
    }
    return 0;
}

int main() {
    CEthread_t* hilos[NUM_HILOS];  // Array de punteros
    
    //CEmutex_init(&mutex1);
    //CEmutex_init(&mutex2);

    short parametro_W = 2;
    int quantum = 10000;
    short algoritmo_calendarizacion = SJF;
    set_algoritmo_calendarizacion_CEthread(algoritmo_calendarizacion);
    set_quantum_CEthread(quantum);

    //set_lado_en_cambio_contexto_RR(LADO_DERECHO);  // Para el algoritmo RR, se necesita indicar la cola con la que se trabajará, pues tiene cambio de contexto
    CEthread_create(&hilos[0], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_DERECHO);
    CEthread_create(&hilos[1], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_DERECHO);
    CEthread_create(&hilos[2], NULL, incrementar2, NULL, PRIORITY_NORMAL, LADO_IZQUIERDO);
    CEthread_create(&hilos[3], NULL, incrementar2, NULL, PRIORITY_NORMAL, LADO_IZQUIERDO);
    CEthread_create(&hilos[4], NULL, incrementar2, NULL, PRIORITY_EMERGENCY, LADO_IZQUIERDO);
    
    CEthread_queue_t* cola_izquierda = get_cola_izquierda();
    CEthread_queue_t* cola_derecha = get_cola_derecha();
    CEthread_t** hilo_actual_izquierda = get_hilo_actual_izquierda();
    CEthread_t** hilo_actual_derecha = get_hilo_actual_derecha();

    inicializar_parametros_flujo(hilo_actual_izquierda, hilo_actual_derecha, cola_izquierda, cola_derecha);
    control_flujo(parametro_W, 0.2, FLUJO_LETRERO, algoritmo_calendarizacion);
    
    // Resultados
    //printf("Valor FINAL: %d (debería ser %d)\n", contador_compartido1, NUM_HILOS * ITERACIONES);
    //printf("Valor FINAL: %d (debería ser %d)\n", contador_compartido2, NUM_HILOS * ITERACIONES);
    printf("Valor FINAL: %d \n", contador_compartido1);
    printf("Valor FINAL: %d \n", contador_compartido2);

    //CEmutex_destroy(&mutex1);
    //CEmutex_destroy(&mutex2);
    return 0;
}


// Compilar código:
// gcc CEThread_utils.c Calendarizador.c CEThread.c control_flujo.c prueba_flujo.c -o prueba_flujo
