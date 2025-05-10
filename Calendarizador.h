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

void queue_init(CEthread_queue_t* q);

void enqueue(CEthread_queue_t* q, CEthread_t* thread);

CEthread_t* dequeue(CEthread_queue_t* q);


////////////////////// ALGORITMOS DE CALENDARIZACION ////////////////////////////////

void calendarizacion_siguiente(CEthread_t** hilo_actual, CEthread_queue_t* q);
void calendarizacion_siguiente_FCFS(CEthread_t** hilo_actual, CEthread_queue_t* q);
void calendarizacion_siguiente_PRIORITY(CEthread_t** hilo_actual, CEthread_queue_t* q);
void calendarizacion_siguiente_SJF(CEthread_t** hilo_actual, CEthread_queue_t* q);
void calendarizacion_siguiente_RR(CEthread_t** hilo_actual, CEthread_queue_t* q);
void cambio_contexto_RR();
void reiniciar_timer();
void detener_timer();
