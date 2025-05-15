#ifndef CETHREAD_H    //Evita inclusiones múltiples. Si otro archivo ya incluyó este .h, no lo volverá a procesar.
#define CETHREAD_H
#define _GNU_SOURCE 
#include <linux/sched.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "CEThread_utils.h"
#include <setjmp.h>

#define MAX_THREADS 50


static const int clone_flags= CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND ;


// Se define el tamaño del stack
static const int STACK_SIZE = 65536;

typedef struct CEthread_attr 
{
	int	    flags;
	void*   stackaddr_attr;
	size_t  stacksize_attr;
} CEthread_attr_t;


// Posibles estados del hilo
typedef enum 
{   
    READY,
    RUNNING,
    BLOCKED,
    FINISHED,
    DETACHED
}CEthread_state;

//Se define la funcion de CEthread
typedef struct CEthread
{
    pid_t                   thread_id;
    CEthread_state           state;
    CEthread_attr_t*        attributes;
    int priority;
    int burst_time;
    jmp_buf context;
    short lado_calle;
} CEthread_t;

// Se crea una cola para los hilos que estén listo para ejecución
typedef struct CEthread_queue {
    CEthread_t* threads[MAX_THREADS];  // Array de punteros a hilos
    int front;                        // Índice del primer hilo listo
    int rear;                         // Índice del último hilo listo
    int count;                        // Número de hilos en la cola
} CEthread_queue_t;


int sannity_check();


// Crear  un thread
pid_t CEthread_create(CEthread_t** CEthread_ptr,
                    CEthread_attr_t** CEthread_attr, 
                    int (*target_function) (void*), 
                    void* args,
                    int priority,
                    short lado_calle);

//Sale del proceso y pone el thread como listo 
int CEthread_end(void* args);

//Retorna el thread
int CEthread_yield();

//Espera a que finalicen todos los thread a los que se puede unir
int CEthread_join(CEthread_t* CEthread_ptr);

// Hacer que un thread no se pueda unir
int CEthread_detach(CEthread_t* CEthread_ptr);


void add_thread(pid_t target, CEthread_t** CEthread_ptr);
size_t search_target_pid(pid_t pid);
void set_lado_en_cambio_contexto_RR (short lado_calle);
void set_detener_timer_RR();
void set_reiniciar_timer_RR();
void set_algoritmo_calendarizacion_CEthread(short algoritmo);
CEthread_queue_t* get_cola_izquierda();
CEthread_queue_t* get_cola_derecha();
CEthread_t** get_hilo_actual_izquierda();
CEthread_t** get_hilo_actual_derecha();
short* get_flag_hilo_actual_actualizado_CEthread();
void set_flag_hilo_actual_actualizado_CEthread(short flag);
void set_quantum_CEthread(int valor_quantum);
volatile short* get_flag_cambio_contexto_CEthread();
void set_flag_cambio_contexto_CEthread(short flag_cambio_contexto);

// Modificar la estructura del mutex
typedef struct CEmutex {
    atomic_int locked;
    pid_t owner;
} CEmutex_t;

// Funciones de mutex
int CEmutex_init(CEmutex_t* mutex);
int CEmutex_destroy(CEmutex_t* mutex);
int CEmutex_lock(CEmutex_t* mutex);
int CEmutex_unlock(CEmutex_t* mutex);

// Esquema de Prioridades
#define PRIORITY_EMERGENCY 0   // Máxima prioridad (vehículos de emergencia)
#define PRIORITY_SPORTS    1   // Prioridad media (vehículos deportivos)
#define PRIORITY_NORMAL    2   // Prioridad baja (vehículos normales)

// Tiempos estimados para SJF (en unidades relativas)
#define EMERGENCY_TIME 1    // Vehículos de emergencia (más rápido)
#define SPORTS_TIME    5    // Vehículos deportivos
#define NORMAL_TIME    10   // Vehículos normales (más lento)

#endif