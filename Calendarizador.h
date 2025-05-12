#include <stdio.h>
#include "CEThread.h"
#include <sys/time.h>  // Para struct itimerval y setitimer()


typedef enum {
    ROUND_ROBIN,
    PRIORITY,
    SJF,
    FCFS,
    REAL_TIME
} Algoritmos_calendarizacion;

void queue_init(CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha);

void enqueue(CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha, CEthread_t* thread);

CEthread_t* dequeue(short lado_calle,CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha);

// Lado de la calle (en unidades relativas)
#define LADO_IZQUIERDO 0   // Vehículos de emergencia (más rápido)
#define LADO_DERECHO   1    // Vehículos deportivos

////////////////////// ALGORITMOS DE CALENDARIZACION ////////////////////////////////

void calendarizacion_siguiente(short lado_calle,CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha);
void calendarizacion_siguiente_FCFS(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha);
void calendarizacion_siguiente_PRIORITY(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha);
void calendarizacion_siguiente_SJF(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha);
void calendarizacion_siguiente_RR(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha);
void calendarizacion_siguiente_REALTIME(CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha);

// Funciones propias del algoritmo FCFS
void aux_calendarizacion_SJF(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q);

// Funciones propias del algoritmo PRIORITY
void aux_calendarizacion_PRIORITY(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q);

// Funciones propias del algortimo Round Robin
void cambio_contexto_RR();
void aux_cambio_contexto_RR(CEthread_t** hilo_actual_ref, CEthread_queue_t* queue_ref);
void reiniciar_timer();
void detener_timer();
void cambiar_lado_contexto_RR (short lado_calle);
