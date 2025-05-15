#include "Calendarizador.h"
#include "CEThread.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>  // Para struct itimerval y setitimer()
#include <linux/sched.h> 

struct itimerval timer; // Usado solo para el algoritmo ROUND ROBIN y RM
short lado_cambio_contexto;  // indica de la cual cola (lado de la calle) proviene o se aplica el cambio de contexto (ROUND ROBIN)

Algoritmos_calendarizacion algoritmo_calendarizacion = ROUND_ROBIN;
CEthread_t** hilo_actual_izquierda_ref = NULL;  // necesario para el algoritmo Round Robin y tiempo real
CEthread_queue_t* queue_izquierda_ref = NULL;    // necesario para el algoritmo Round Robin y tiempo real
CEthread_t** hilo_actual_derecha_ref = NULL;  // necesario para el algoritmo Round Robin y tiempo real
CEthread_queue_t* queue_derecha_ref = NULL;    // necesario para el algoritmo Round Robin y tiempo real
short flag_hilo_actual_actualizado = 0;
int quantum = 1;
volatile short flag_RR_cambio_contexto = 0;

///////////////////////////////// COLA DE HILOS LISTOS //////////////////////////

void queue_init(CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha){
    q_izquierda->front = 0;
    q_izquierda->rear = -1;
    q_izquierda->count = 0;

    q_derecha->front = 0;
    q_derecha->rear = -1;
    q_derecha->count = 0;
    queue_izquierda_ref = q_izquierda;  // se almacena una referencia a la cola (utilizada en RR y tiempo real)
    queue_derecha_ref = q_derecha;  // se almacena una referencia a la cola (utilizada en RR y tiempo real)
}

void enqueue(CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha, CEthread_t* thread){
    if (q_izquierda->count >= MAX_THREADS && q_derecha->count >= MAX_THREADS) {
        fprintf(stderr, "Error: Cola de hilos llena\n");
        exit(EXIT_FAILURE);
    }

    // Cuando el hilo debe ir en la cola del lado izquierdo (lado izquierdo de la calle)
    if (thread->lado_calle == LADO_IZQUIERDO){
        q_izquierda->rear = (q_izquierda->rear + 1) % MAX_THREADS;
        q_izquierda->threads[q_izquierda->rear] = thread;
        q_izquierda-> threads[(q_izquierda->rear + 1) % MAX_THREADS] = NULL;  // Se asegura que el siguient espacio en la cola sea nulo, pues si se alcanzan más de 50, podría haber residuos
        thread->state = READY;
        q_izquierda->count++;
    }
    // Cuando el hilo debe ir en la cola del lado derecho (lado izquierdo de la calle)
    else{
        q_derecha->rear = (q_derecha->rear + 1) % MAX_THREADS;
        q_derecha->threads[q_derecha->rear] = thread;
        q_derecha-> threads[(q_derecha->rear + 1) % MAX_THREADS] = NULL;  // Se asegura que el siguient espacio en la cola sea nulo, pues si se alcanzan más de 50, podría haber residuos
        thread->state = READY;
        q_derecha->count++;

    }
}

CEthread_t* dequeue(short lado_calle, CEthread_queue_t* q_izquierda, CEthread_queue_t* q_derecha){

    // Se obtiene un thread de la cola de la izquierda
    if (q_izquierda->count <= 0 && lado_calle == LADO_IZQUIERDO) {
        return NULL;
    }
    else if (lado_calle == LADO_IZQUIERDO) {
        CEthread_t* thread = q_izquierda->threads[q_izquierda->front];
        q_izquierda->front = (q_izquierda->front + 1) % MAX_THREADS;
        q_izquierda->count--;
        return thread;
    }

     // Se obtiene un thread de la cola de la derecha
    if (q_derecha->count <= 0 && lado_calle == LADO_DERECHO) {
        return NULL;
    }
    else if (lado_calle == LADO_DERECHO) {
        CEthread_t* thread = q_derecha->threads[q_derecha->front];
        q_derecha->front = (q_derecha->front + 1) % MAX_THREADS;
        q_derecha->count--;
        return thread;
    }    
}


//////////////////////////////////////////////// ALGORITMOS DE CALENDARIZACION ////////////////////////////////


// Dependiendo de cual sea el algoritmo escogido por el usuario se llama dicha calendarización
void calendarizacion_siguiente(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha){
    switch (algoritmo_calendarizacion) {
        case FCFS:
            calendarizacion_siguiente_FCFS(lado_calle, hilo_actual_t, q_izquierda, q_derecha);
            break;
        case PRIORITY:
            calendarizacion_siguiente_PRIORITY(lado_calle, hilo_actual_t, q_izquierda, q_derecha);
            break;
        case SJF:
            calendarizacion_siguiente_SJF(lado_calle, hilo_actual_t, q_izquierda, q_derecha);
            break;
        case ROUND_ROBIN:
            calendarizacion_siguiente_RR(lado_calle, hilo_actual_t, q_izquierda, q_derecha);
            break;
        case REAL_TIME:

            break;
        default:
            calendarizacion_siguiente_FCFS(lado_calle, hilo_actual_t, q_izquierda, q_derecha);
    }
}

void calendarizacion_siguiente_FCFS(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha) {
    
    if (lado_calle == LADO_IZQUIERDO){
        // Obtener el siguiente hilo (FIFO) de la cola de la izquierda
        *hilo_actual_t = dequeue(lado_calle, q_izquierda, q_derecha);
    }
    else{
        // Obtener el siguiente hilo (FIFO) de la cola de la derecho
        *hilo_actual_t = dequeue(lado_calle, q_izquierda, q_derecha);
    }

    /*
    // Reanudar el nuevo hilo
    if (*hilo_actual_t != NULL){
        //printf("PASA POR AQUI ANTES DE PETAR CUANTAS VECES \n");
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        (*hilo_actual_t)->state = RUNNING;
    }
    */
}


////////////////////////////////////////////////// ALGORITMO PRIORITY ////////////////////////////////////////////
// Implementación del algoritmo de Calendarización por Prioridad

void calendarizacion_siguiente_PRIORITY(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha) {

    if (lado_calle == LADO_IZQUIERDO) {
        aux_calendarizacion_PRIORITY(lado_calle, hilo_actual_t, q_izquierda);
    }
    else{
        aux_calendarizacion_PRIORITY(lado_calle, hilo_actual_t, q_derecha);
    }

    /*
    //Activación del hilo seleccionado
    if (*hilo_actual_t != NULL) {
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        (*hilo_actual_t)->state = RUNNING;
    }
    */
}


void aux_calendarizacion_PRIORITY(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q){

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
        *hilo_actual_t = dequeue(lado_calle, q, q);  // se mandan las dos colas iguales, pero solo se usa la que se necesite (depnediendo de 'lado_calle')
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
}


////////////////////////////////////////////////////// ALGORITMO SJF ////////////////////////////////////////////
// Implementación del algoritmo de Calendarización SJF

void calendarizacion_siguiente_SJF(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha) {
    
    if (lado_calle == LADO_IZQUIERDO) {
        aux_calendarizacion_SJF(lado_calle, hilo_actual_t, q_izquierda);
    }
    else{
        aux_calendarizacion_SJF(lado_calle, hilo_actual_t, q_derecha);
    }

    /*
    if (*hilo_actual_t != NULL) {
        kill((*hilo_actual_t)->thread_id, SIGCONT);
        (*hilo_actual_t)->state = RUNNING;
    }
    */
}

void aux_calendarizacion_SJF(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q){
    
    // necesaria para cuando ya no hay hilos que ejecutar, sino el programa no terminaría (enciclado)
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
        *hilo_actual_t = dequeue(lado_calle, q, q);
    
    } else {
        for (int i = selected_index; i != q->rear; i = (i + 1) % MAX_THREADS) {
            q->threads[i] = q->threads[(i + 1) % MAX_THREADS];
        }
        
        q->rear = (q->rear - 1 + MAX_THREADS) % MAX_THREADS;
        q->count--;
        
        *hilo_actual_t = selected_thread;
    }
}


////////////////////////////////////////////////////// ALGORITMO ROUND ROBIN ////////////////////////////////////////////
// Implementación del algoritmo de Calendarización Round Robin

void calendarizacion_siguiente_RR(short lado_calle, CEthread_t** hilo_actual_t, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha){

    // Se configura la calendarización al inicio
    if (*hilo_actual_t == NULL && hilo_actual_izquierda_ref == NULL && hilo_actual_derecha_ref == NULL){

        timer.it_value.tv_sec = 0;          // Segundos iniciales (0)
        timer.it_value.tv_usec = quantum;     // Microsegundos iniciales (10 ms = 10000 us)
        timer.it_interval = timer.it_value; // Intervalo = Mismo valor (periódico)
        setitimer(ITIMER_REAL, &timer, NULL); // Inicia el timer
        //signal(SIGALRM, cambio_contexto_RR);     // Asocia SIGALRM al manejador
        
    }
    detener_timer(); // se debe de analizar, porque si una cola está en ejecución, el timer no puede detenerse

    if (lado_calle == LADO_IZQUIERDO){
        *hilo_actual_t = dequeue(lado_calle, q_izquierda, q_derecha);
        hilo_actual_izquierda_ref = hilo_actual_t;
    }
    else{
        *hilo_actual_t = dequeue(lado_calle, q_izquierda, q_derecha);
        hilo_actual_derecha_ref = hilo_actual_t;
    }

    /*
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
    */
}

void cambio_contexto_RR(){
    
    detener_timer();

    if (lado_cambio_contexto == LADO_IZQUIERDO){
        aux_cambio_contexto_RR(hilo_actual_izquierda_ref, queue_izquierda_ref);
    }
    else{
        aux_cambio_contexto_RR(hilo_actual_derecha_ref, queue_derecha_ref);
    }

    flag_hilo_actual_actualizado = 1;
    flag_RR_cambio_contexto = 1;
     // Si hilo_a_ejecutar fuera NULL, no hay cambio de contexto, el hilo atual puede seguir ejecutando lo que estaba haciendo
    //reiniciar_timer();
}

void aux_cambio_contexto_RR(CEthread_t** hilo_actual_ref, CEthread_queue_t* queue_ref){
    CEthread_t *hilo_a_ejecutar = dequeue(lado_cambio_contexto, queue_ref, queue_ref);

    if (hilo_a_ejecutar != NULL){
        // Se da un cambio de contexto
        kill((*hilo_actual_ref)->thread_id, SIGSTOP);  // se detiene la ejecución del hilo actual, pues su tiempo (quantum) ya se acabó
        (*hilo_actual_ref)->state = BLOCKED;
        enqueue(queue_ref, queue_ref, *hilo_actual_ref);  // se agrega a la cola el hilo actual, pues todavia tiene cosas pendientes que ejecutar
        *hilo_actual_ref = hilo_a_ejecutar;  // se cambia el hilo actual al hilo que se saca de la cola (pues se le cede el CPU)
        
        /*
        (*hilo_actual_ref)->state = RUNNING;
        kill((*hilo_actual_ref)->thread_id, SIGCONT);  // se comienza su ejecución
        */
    }
}

// De tiene el timer que controla el SIGALRM
void detener_timer(){
    memset(&timer, 0, sizeof(timer));  // Configura todo a 0
    setitimer(ITIMER_REAL, &timer, NULL);  // (detiene el timer)
}

// Reinicia el timer que controla el SIGARLM
void reiniciar_timer(){
    // Reiniciar el timer para el próximo hilo (con 10 ms)
    timer.it_value.tv_sec = 0; 
    timer.it_value.tv_usec = quantum;     // 10 ms
    timer.it_interval = timer.it_value;  // Periodicidad
    setitimer(ITIMER_REAL, &timer, NULL);  // Inicia el timer
    signal(SIGALRM, cambio_contexto_RR);     // Asocia SIGALRM al manejador
}

void cambiar_lado_contexto_RR (short lado_calle){
    lado_cambio_contexto = lado_calle;  // asigna el lado de la calle (o la cola específica) al que corresponde el cambio de contexto que se va a ejecutar
}

void set_algoritmo_calendarizacion(Algoritmos_calendarizacion algoritmo){
    algoritmo_calendarizacion = algoritmo; 
}

short* get_flag_hilo_actual_actualizado(){
    return  &flag_hilo_actual_actualizado;
}

void set_flag_hilo_actual_actualizado(short flag){
    flag_hilo_actual_actualizado = flag;
}

void set_quantum(int valor_quantum){
    quantum = valor_quantum;
}

volatile short* get_flag_cambio_contexto(){
    return &flag_RR_cambio_contexto;
}

void set_flag_cambio_contexto(short flag_cambio_contexto){
    flag_RR_cambio_contexto = flag_cambio_contexto;
}

////////////////////////////////////////////////////// ALGORITMO DE TIEMPO REAL RM ////////////////////////////////////////////
// Implementación del algoritmo de Calendarización RM

void calendarizacion_siguiente_REALTIME(CEthread_t** hilo_actual, CEthread_queue_t* q_izquierda,  CEthread_queue_t* q_derecha){

}



// Calcular los datos de la tabla de calendarizacion para cada algoritmo (como en clase)
// https://github.com/AugustineAykara/CPU-Scheduling-Algorithm-In-C

// Referencia
// https://github.com/DARD172002/CEThreads/blob/master/library/CEThreads.c