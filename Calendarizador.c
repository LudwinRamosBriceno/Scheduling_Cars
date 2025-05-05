#include "Calendarizador.h"
#include "CEThread.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>


Algoritmos_calendarizacion algoritmo = PRIORITY;

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
void calendarizacion_siguiente(CEthread_t** hilo_actual_t, CEthread_queue_t* q) {
    switch (algoritmo) {
        case FCFS:
            calendarizacion_siguiente_FCFS(hilo_actual_t, q);
            break;
        case PRIORITY:
            calendarizacion_siguiente_PRIORITY(hilo_actual_t, q);
            break;
        default:
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

// Implementación del algoritmo de prioridad

void calendarizacion_siguiente_PRIORITY(CEthread_t** hilo_actual_t, CEthread_queue_t* q) {

    // Comprueba si no hay hilos en la cola de listos.
    // Si la cola está vacía, devuelve NULL y termina.
    if (q->count <= 0) {
        *hilo_actual_t = NULL;
        return;
    }

    // Inicialización de variables clave
    int lowest_priority = 3;  // Valor inicial para hacer verdadera la comparación de búsqueda
    int selected_index = -1;
    
    // Búsqueda del hilo con mayor prioridad (número más bajo)
    for (int i = 0; i < q->count; i++) {
        int current_index = (q->front + i) % MAX_THREADS;
        CEthread_t* current_thread = q->threads[current_index];
        
        // Buscamos el valor MÁS PEQUEÑO (prioridad más alta)
        if (current_thread->priority < lowest_priority) {
            lowest_priority = current_thread->priority;
            selected_index = current_index;
        }
    }

    // Verifica que realmente se encontró un hilo válido
    if (selected_index == -1) {
        *hilo_actual_t = NULL;
        return;
    }

    // Extracción del hilo seleccionado
    CEthread_t* selected_thread = q->threads[selected_index];
    
    // Caso simple: hilo al frente (Si está al frente, lo extrae directamente.)
    if (selected_index == q->front) {
        *hilo_actual_t = dequeue(q);
    } 
    //Caso complejo: hilo en medio
    // Desplaza los elementos posteriores para "llenar el hueco"
    // Ajusta los índices de la cola circular
    else {
        for (int i = selected_index; i != q->rear; i = (i + 1) % MAX_THREADS) {
            q->threads[i] = q->threads[(i + 1) % MAX_THREADS];
        }
        
        q->rear = (q->rear - 1 + MAX_THREADS) % MAX_THREADS;
        q->count--;
        
        *hilo_actual_t = selected_thread;
    }
    
    //Activación del hilo seleccionado
    if (*hilo_actual_t != NULL) {
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        (*hilo_actual_t)->state = RUNNING;
    }
}


// Calcular los datos de la tabla de calendarizacion para cada algoritmo (como en clase)
// https://github.com/AugustineAykara/CPU-Scheduling-Algorithm-In-C

// Referencia
// https://github.com/DARD172002/CEThreads/blob/master/library/CEThreads.c