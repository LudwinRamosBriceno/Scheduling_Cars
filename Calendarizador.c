#include "Calendarizador.h"
#include "CEThread.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>


Algoritmos_calendarizacion algoritmo = FCFS;

///////////////////////////////// COLA DE HILOS LISTOS //////////////////////////

void queue_init(CEthread_queue_t* q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

void enqueue(CEthread_queue_t* q, CEthread_t* thread) {
    if (q->count >= MAX_THREADS) {
        fprintf(stderr, "Error: Cola de hilos llena\n");
        exit(EXIT_FAILURE);
    }
    q->rear = (q->rear + 1) % MAX_THREADS;
    q->threads[q->rear] = thread;
    thread->state = READY;
    q->count++;
}

CEthread_t* dequeue(CEthread_queue_t* q) {
    if (q->count <= 0) {
        return NULL;  // No hay hilos listos
    }
    CEthread_t* thread = q->threads[q->front];
    q->front = (q->front + 1) % MAX_THREADS;
    q->count--;
    return thread;
}


////////////////////// ALGORITMOS DE CALENDARIZACION ////////////////////////////////


// Dependiendo de cual sea el algoritmo escogido por el usuario se llama dicha calendarización
void calendarizacion_siguiente(CEthread_t** hilo_actual_t, CEthread_queue_t* q){

    if (algoritmo == FCFS){
        calendarizacion_siguiente_FCFS(hilo_actual_t, q);
    }
}

void calendarizacion_siguiente_FCFS(CEthread_t** hilo_actual_t, CEthread_queue_t* q) {
    
    // Obtener el siguiente hilo (FIFO)
    *hilo_actual_t = dequeue(q);

    // Reanudar el nuevo hilo
    if (*hilo_actual_t != NULL){
        //printf("PASA POR AQUI ANTES DE PETAR CUANTAS VECES \n");
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        (*hilo_actual_t)->state = RUNNING;
    }
}


// Calcular los datos de la tabla de calendarizacion para cada algoritmo (como en clase)
// https://github.com/AugustineAykara/CPU-Scheduling-Algorithm-In-C

// Referencia
// https://github.com/DARD172002/CEThreads/blob/master/library/CEThreads.c