#include <stdio.h>
#include <stdlib.h>
#include "CEThread.h"
#include "control_flujo.h"

#define NUM_HILOS 6
#define ITERACIONES 20000000

#define LADO_IZQUIERDO 0
#define LADO_DERECHO   1
#define LADO_GENERAL_IZQUIERDO 2   
#define LADO_GENERAL_DERECHO   3      


int contador_compartido1 = 0;
int contador_compartido2 = 0;
CEmutex_t mutex1;
CEmutex_t mutex2;

void prueba_equidad(Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion_test, short param_W_test, int quantum_test, double cambio_letrero_test);
void prueba_letrero(Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion_test, short param_W_test ,int quantum_test, double cambio_letrero_test);
void prueba_FIFO(Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion_test, short param_W_test ,int quantum_test, double cambio_letrero_test);


int incrementar1() {
    for (int i = 0; i < ITERACIONES; i++) {
        CEmutex_lock(&mutex1);  
        contador_compartido1++;
        CEmutex_unlock(&mutex1); 
    }
    return 0;
}

int incrementar2() {
    for (int i = 0; i < ITERACIONES; i++) {
        //CEmutex_lock(&mutex2);  
        contador_compartido2++;
        //CEmutex_unlock(&mutex2); 
    }
    return 0;
}

int main() {

    //CEmutex_init(&mutex1);
    //CEmutex_init(&mutex2);

    short parametro_W = 2;
    int quantum = 10000; // (10 ms) 
    double cambio_letrero = 0.2;  // 200 ms
    int algoritmo_calendarizacion = PRIORITY;

    //prueba_equidad(algoritmo_calendarizacion, parametro_W, quantum, cambio_letrero);
    prueba_letrero(algoritmo_calendarizacion, parametro_W, quantum, cambio_letrero);
    //prueba_FIFO(algoritmo_calendarizacion, parametro_W, quantum, cambio_letrero);

    // Resultados
    printf("Valor FINAL para Equidad: %d \n", contador_compartido1);
    printf("Valor FINAL para Equidad: %d \n", contador_compartido2);

    //CEmutex_destroy(&mutex1);
    //CEmutex_destroy(&mutex2);
    return 0;
}

void prueba_equidad(Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion_test, short param_W_test, int quantum_test, double cambio_letrero_test){
    CEthread_t* hilos[NUM_HILOS];  // Array de punteros
    
    short parametro_W = param_W_test;
    int quantum = quantum_test; // (10 ms) 
    double cambio_letrero = cambio_letrero_test;  // 200 ms
    short algoritmo_calendarizacion = algoritmo_calendarizacion_test;
    set_algoritmo_calendarizacion_CEthread(algoritmo_calendarizacion);
    set_quantum_CEthread(quantum);

    // PRUEBA PARA EQUIDAD
    CEthread_create(&hilos[0], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_DERECHO);
    CEthread_create(&hilos[1], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_DERECHO);
    CEthread_create(&hilos[2], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_DERECHO);
    CEthread_create(&hilos[3], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_IZQUIERDO);
    CEthread_create(&hilos[4], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_IZQUIERDO);
    CEthread_create(&hilos[5], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_IZQUIERDO);

    // incialización de colas y punteros del hilo actual (solo se hace una vez)
    CEthread_queue_t* cola_izquierda = get_cola_izquierda();
    CEthread_queue_t* cola_derecha = get_cola_derecha();
    CEthread_queue_t* cola_general = get_cola_general();
    CEthread_t** hilo_actual_izquierda = get_hilo_actual_izquierda();
    CEthread_t** hilo_actual_derecha = get_hilo_actual_derecha();
    CEthread_t** hilo_actual_general = get_hilo_actual_general();

    // se inicializan los parámetros
    inicializar_parametros_flujo(hilo_actual_izquierda, hilo_actual_derecha, hilo_actual_general, cola_izquierda, cola_derecha, cola_general);

    control_flujo(parametro_W, cambio_letrero, FLUJO_EQUIDAD, algoritmo_calendarizacion); // se ejecuta la prueba
}


void prueba_letrero(Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion_test, short param_W_test ,int quantum_test, double cambio_letrero_test){
    CEthread_t* hilos[NUM_HILOS];  // Array de punteros
    
    short parametro_W = param_W_test;
    int quantum = quantum_test; // (10 ms) 
    double cambio_letrero = cambio_letrero_test;  // 200 ms
    short algoritmo_calendarizacion = algoritmo_calendarizacion_test;
    set_algoritmo_calendarizacion_CEthread(algoritmo_calendarizacion);
    set_quantum_CEthread(quantum);

    // PRUEBA PARA LETRERO
    CEthread_create(&hilos[0], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_DERECHO);
    CEthread_create(&hilos[1], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_DERECHO);
    CEthread_create(&hilos[2], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_DERECHO);
    CEthread_create(&hilos[3], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_IZQUIERDO);
    CEthread_create(&hilos[4], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_IZQUIERDO);
    CEthread_create(&hilos[5], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_IZQUIERDO);

    // incialización de colas y punteros del hilo actual (solo se hace una vez)
    CEthread_queue_t* cola_izquierda = get_cola_izquierda();
    CEthread_queue_t* cola_derecha = get_cola_derecha();
    CEthread_queue_t* cola_general = get_cola_general();
    CEthread_t** hilo_actual_izquierda = get_hilo_actual_izquierda();
    CEthread_t** hilo_actual_derecha = get_hilo_actual_derecha();
    CEthread_t** hilo_actual_general = get_hilo_actual_general();

    // se inicializan los parámetros
    inicializar_parametros_flujo(hilo_actual_izquierda, hilo_actual_derecha, hilo_actual_general, cola_izquierda, cola_derecha, cola_general);

    control_flujo(parametro_W, cambio_letrero, FLUJO_LETRERO, algoritmo_calendarizacion); // se ejecuta la prueba

}

void prueba_FIFO(Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion_test, short param_W_test ,int quantum_test, double cambio_letrero_test){
    CEthread_t* hilos[NUM_HILOS];  // Array de punteros
    
    short parametro_W = param_W_test;
    int quantum = quantum_test; // (10 ms) 
    double cambio_letrero = cambio_letrero_test;  // 200 ms
    short algoritmo_calendarizacion = algoritmo_calendarizacion_test;
    set_algoritmo_calendarizacion_CEthread(algoritmo_calendarizacion);
    set_quantum_CEthread(quantum);

    // PRUEBA PARA FIFO
    CEthread_create(&hilos[0], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_GENERAL_DERECHO);
    CEthread_create(&hilos[1], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_GENERAL_DERECHO);
    CEthread_create(&hilos[2], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_GENERAL_DERECHO);
    CEthread_create(&hilos[3], NULL, incrementar1, NULL, PRIORITY_NORMAL, LADO_GENERAL_IZQUIERDO);
    CEthread_create(&hilos[4], NULL, incrementar1, NULL, PRIORITY_SPORTS, LADO_GENERAL_IZQUIERDO);
    CEthread_create(&hilos[5], NULL, incrementar1, NULL, PRIORITY_EMERGENCY, LADO_GENERAL_IZQUIERDO);

    // incialización de colas y punteros del hilo actual (solo se hace una vez)
    CEthread_queue_t* cola_izquierda = get_cola_izquierda();
    CEthread_queue_t* cola_derecha = get_cola_derecha();
    CEthread_queue_t* cola_general = get_cola_general();
    CEthread_t** hilo_actual_izquierda = get_hilo_actual_izquierda();
    CEthread_t** hilo_actual_derecha = get_hilo_actual_derecha();
    CEthread_t** hilo_actual_general = get_hilo_actual_general();

    // se inicializan los parámetros
    inicializar_parametros_flujo(hilo_actual_izquierda, hilo_actual_derecha, hilo_actual_general, cola_izquierda, cola_derecha, cola_general);

    control_flujo(parametro_W, cambio_letrero, FLUJO_FIFO, algoritmo_calendarizacion); // se ejecuta la prueba
}

// Compilar código:
// gcc CEThread_utils.c Calendarizador.c CEThread.c control_flujo.c prueba_flujo.c -o prueba_flujo