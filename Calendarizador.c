#define _GNU_SOURCE
#include "Calendarizador.h"
#include "CEThread.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>  // Para struct itimerval y setitimer()
#include <linux/sched.h>
#include <setjmp.h>


struct itimerval timer;

Algoritmos_calendarizacion algoritmo = ROUND_ROBIN;
CEthread_t** hilo_actual_ref = NULL;  // necesario para el algoritmo Round Robin y tiempo real
CEthread_queue_t* queue_ref = NULL;    // necesario para el algoritmo Round Robin y tiempo real

///////////////////////////////// COLA DE HILOS LISTOS //////////////////////////

void queue_init(CEthread_queue_t* q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    queue_ref = q;  // se almacena una referencia a la cola (utilizada en RR y tiempo real)
}

void enqueue(CEthread_queue_t* q, CEthread_t* thread) {
    if (q->count >= MAX_THREADS) {
        fprintf(stderr, "Error: Cola de hilos llena\n");
        exit(EXIT_FAILURE);
    }
    q->rear = (q->rear + 1) % MAX_THREADS;
    q->threads[q->rear] = thread;
    q-> threads[(q->rear + 1) % MAX_THREADS] = NULL;  // Se asegura que el siguient espacio en la cola sea nulo, pues si se alcanzan más de 50, podría haber residuos
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
        case SJF:
            calendarizacion_siguiente_SJF(hilo_actual_t, q);
            break;
        case ROUND_ROBIN:
            calendarizacion_siguiente_RR(hilo_actual_t, q);
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

// Implementación del algoritmo de Calendarización por Prioridad

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

// Implementación del algoritmo de Calendarización SJF

void calendarizacion_siguiente_SJF(CEthread_t** hilo_actual_t, CEthread_queue_t* q) {
    if (q->count <= 0) {
        *hilo_actual_t = NULL;
        return;
    }

    // Inicializar con el primer hilo de la cola
    int shortest_time = q->threads[q->front]->burst_time;
    int selected_index = q->front;
    
    // Comenzar la búsqueda desde el segundo hilo (si existe)
    for (int i = 1; i < q->count; i++) {
        int current_index = (q->front + i) % MAX_THREADS;
        CEthread_t* current_thread = q->threads[current_index];
        
        if (current_thread->burst_time < shortest_time) {
            shortest_time = current_thread->burst_time;
            selected_index = current_index;
        }
    }

    CEthread_t* selected_thread = q->threads[selected_index];
    
    if (selected_index == q->front) {
        *hilo_actual_t = dequeue(q);
    } else {
        for (int i = selected_index; i != q->rear; i = (i + 1) % MAX_THREADS) {
            q->threads[i] = q->threads[(i + 1) % MAX_THREADS];
        }
        
        q->rear = (q->rear - 1 + MAX_THREADS) % MAX_THREADS;
        q->count--;
        
        *hilo_actual_t = selected_thread;
    }

    if (*hilo_actual_t != NULL) {
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        (*hilo_actual_t)->state = RUNNING;
    }
}


void calendarizacion_siguiente_RR(CEthread_t** hilo_actual_t, CEthread_queue_t* q){

    // Se configura la calendarización al inicio
    if (*hilo_actual_t == NULL){

        timer.it_value.tv_sec = 0;          // Segundos iniciales (0)
        timer.it_value.tv_usec = 10000;     // Microsegundos iniciales (5 ms)
        timer.it_interval = timer.it_value; // Intervalo = Mismo valor (periódico)
        setitimer(ITIMER_REAL, &timer, NULL); // Inicia el timer
        //signal(SIGALRM, cambio_contexto_RR);     // Asocia SIGALRM al manejador

    }
   
    detener_timer();

    // Obtener el siguiente hilo (FIFO)
    *hilo_actual_t = dequeue(q);
    hilo_actual_ref = hilo_actual_t;  // Se almacena el hilo actual que servirá para el cambio de contexto

    // Comenzar un nuevo hilo ejecutar
    if (*hilo_actual_t != NULL){
        // Se corre el hilo actual
        (*hilo_actual_t)->state = RUNNING;
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        reiniciar_timer();

    }
    else{
        detener_timer();
    }
}

void cambio_contexto_RR(){
    
    //  OPCIÓN 1
    detener_timer();
    CEthread_t *hilo_a_ejecutar = dequeue(queue_ref);

    if (hilo_a_ejecutar != NULL){
        // Se da un cambio de contexto
        //printf("Cede: %d\n", (*hilo_actual_ref)->thread_id);
        kill((*hilo_actual_ref)->thread_id, SIGSTOP);  // se detiene la ejecución del hilo actual, pues su tiempo (quantum) ya se acabó
        (*hilo_actual_ref)->state = BLOCKED;
        enqueue(queue_ref, *hilo_actual_ref);  // se agrega a la cola el hilo actual, pues todavia tiene cosas pendientes que ejecutar
        *hilo_actual_ref = hilo_a_ejecutar;  // se cambia el hilo actual al hilo que se saca de la cola (pues se le cede el CPU)
        (*hilo_actual_ref)->state = RUNNING;
        //printf("Se ejecuta: %d\n", (*hilo_actual_ref)->thread_id);
        kill((*hilo_actual_ref)->thread_id, SIGCONT);  // se comienza su ejecución
    }
     // Si hilo_a_ejecutar fuera NULL, no hay cambio de contexto, el hilo atual puede seguir ejecutando lo que estaba haciendo
    reiniciar_timer();
    
}

void detener_timer(){
    memset(&timer, 0, sizeof(timer));  // Configura todo a 0
    setitimer(ITIMER_REAL, &timer, NULL);  // (detiene el timer)
}

void reiniciar_timer(){
    // Reiniciar el timer para el próximo hilo (con 5 ms)
    timer.it_value.tv_usec = 10000;     // 5 ms
    timer.it_interval = timer.it_value;  // Periodicidad
    setitimer(ITIMER_REAL, &timer, NULL);  // Inicia el timer
    signal(SIGALRM, cambio_contexto_RR);     // Asocia SIGALRM al manejador
}



// Calcular los datos de la tabla de calendarizacion para cada algoritmo (como en clase)
// https://github.com/AugustineAykara/CPU-Scheduling-Algorithm-In-C

// Referencia
// https://github.com/DARD172002/CEThreads/blob/master/library/CEThreads.c