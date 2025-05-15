#ifndef CONTRO_FLUJO_H
#define CONTRO_FLUJO_H
#include "CEThread.h"

typedef enum {
    FLUJO_EQUIDAD,
    FLUJO_LETRERO,
    FLUJO_FIFO
} TipoFlujo;

typedef enum {
    ROUND_ROBIN,
    PRIORITY,
    SJF,
    FCFS,
    REAL_TIME
} Algoritmos_calendarizacion_en_flujo;

void inicializar_parametros_flujo(CEthread_t** hilo_actual_izquierda, CEthread_t** hilo_actual_derecha, 
                                    CEthread_queue_t* cola_listo_izquierda, CEthread_queue_t* cola_listo_derecha);
void control_flujo(short param_W, short tiempoLetrero, TipoFlujo algoritmoFlujo, Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion);
void equidad(short param_W, Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion);
void letrero(short tiempoLetrero);
void FIFO();


void equidad_aux(CEthread_t** hilo_actual, Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion);


#endif


