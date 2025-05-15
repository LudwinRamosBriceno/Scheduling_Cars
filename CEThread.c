#include "CEThread.h"
#include "CEThread_utils.h"
#include "Calendarizador.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h> 
#include <time.h>
#include <stdatomic.h>


// Manejador de Threads
CEthread_t manager_thread;

// Array de threads 

short isInitialized = 0;
pid_t threads_pid[MAX_THREADS];
CEthread_t* threads_ptr[MAX_THREADS];

// Variables globales (o dentro de una estructura de scheduler)
CEthread_queue_t cola_de_listo_izquierda;
CEthread_queue_t cola_de_listo_derecha;

CEthread_t* hilo_actual_izquierda = NULL;    // Hilo actualmente en ejecución en la cola de izquierda
CEthread_t* hilo_actual_derecha = NULL;    // Hilo actualmente en ejecución en la cola de la derecha



/*
 Función que garantiza que se cree una instancia de la lista de procesos y
 comprueba si hay procesos que finalizaron y deben eliminarse de la lista de hilos.
 
 Devoluciones
 int 0 si hay espacios libres en la matriz de hilos, -1 si está llena
 */

 int sannity_check(){

    //Inicializa la matriz de subprocesos en caso de que sea la primera vez que se llama
    if(!isInitialized){

        srand(time(NULL));

        for (size_t i = 0; i < MAX_THREADS; ++i)
        {
            threads_pid[i] = -1;
        }

        isInitialized = 1;
        queue_init(&cola_de_listo_izquierda, &cola_de_listo_derecha);  // Se inicializa los parámetros de la cola de hilos

        return 0;
    }
    return 0;   
}


//Funcion que crea el thread

pid_t CEthread_create(CEthread_t** CEthread_ptr,
                    CEthread_attr_t** CEthread_attr, 
                    int (*target_function) (void*), 
                    void* args,
                    int priority,
                    short lado_calle)
    {

    sannity_check();


    //Asignacion de pila para tareas secundarias.

    char* stack = (char*) allocate_mem(STACK_SIZE);

    // Iniciar el thread
    // Configurar para que se detenga y se inicie cuando la estructura esté lista
    CEthread_arguments_t* clone_args = allocate_mem(sizeof(CEthread_arguments_t));
    clone_args->target_function = target_function;
    clone_args->fnctn_args = args;

    pid_t thread_id = clone_call(CEthread_end, stack + STACK_SIZE, clone_flags, clone_args);

    // Verifica si la creación del hilo fue exitosa
    if (thread_id == -1) {
        perror("Error creando el hilo");
        return -1;  // Indica un error
    } else {
        printf("Hilo creado exitosamente con ID: %d\n", thread_id);
    }
    
    // Asignar memoria para los atributos y la estructura
    CEthread_t* ptr = allocate_mem(sizeof(CEthread_t));
    CEthread_attr_t* attr = allocate_mem(sizeof(CEthread_attr_t));

    
    // Crear una instancia de la estructura CEthread_attr

    attr->flags = clone_flags;
    attr->stackaddr_attr = stack;
    attr->stacksize_attr = STACK_SIZE;

    //Crear una instancia de CEthread
    ptr->thread_id = thread_id;
    ptr->state = BLOCKED;
    ptr->priority = priority;
    ptr->lado_calle = lado_calle;
    ptr->attributes = attr;

    // Asignar el Burst Time de acuerdo con la prioridad del Hilo
    switch(priority) {
        case PRIORITY_EMERGENCY:
            ptr->burst_time = EMERGENCY_TIME;
            break;
        case PRIORITY_SPORTS:
            ptr->burst_time = SPORTS_TIME;
            break;
        case PRIORITY_NORMAL:
            ptr->burst_time = NORMAL_TIME;
            break;
        default:
            ptr->burst_time = NORMAL_TIME;
    }

    //Si tenemos una estructura a la cual hacer referencia entonces actualizar el ptr

    if(CEthread_ptr != NULL){
        *CEthread_ptr = ptr;
    }

    // Lo mismo para attr
    if(CEthread_attr != NULL){
        *CEthread_attr = attr;
    }

    add_thread(thread_id, &ptr);

    // Se añade el hilo a la cola de hilos para calendarización
    enqueue(&cola_de_listo_izquierda, &cola_de_listo_derecha, ptr);
    
    // Si es el primer hilo que ingresa a la cola, iniciarlo
    if (hilo_actual_izquierda == NULL && ptr->lado_calle == LADO_IZQUIERDO) {
        calendarizacion_siguiente(LADO_IZQUIERDO, &hilo_actual_izquierda, &cola_de_listo_izquierda, &cola_de_listo_derecha);
    }
    else if (hilo_actual_derecha == NULL && ptr->lado_calle == LADO_DERECHO) {
        calendarizacion_siguiente(LADO_DERECHO, &hilo_actual_derecha, &cola_de_listo_izquierda, &cola_de_listo_derecha);
    }
    
    return thread_id;
}


//Funcion para eliminar el thread

int CEthread_end(void* args){
    CEthread_arguments_t* clone_args = (CEthread_arguments_t*) args;
    //Ejecuta la funcion con los argumentos dados
    clone_args->target_function(clone_args->fnctn_args);
    //Libera memoria
    free(clone_args);
    
    //Obtener el id del thread
    int pid = getpid();

    // Luego sale de forma segura
    //Busca el pid objetivo
    
    for (size_t i = 0; i < MAX_THREADS; i++){
        
        if(threads_pid[i] == pid){
            //printf("Hilo con PID %d ha finalizado.\n", pid);
            
            //si es -2, lo que significa es que hay memoria libre para reutilizar
            threads_pid[i] = -2;
            // cambio del estados a terminado
            printf("Hilo con PID %d ha finalizado.\n", pid);
            threads_ptr[i]->state = FINISHED;
            
            switch (threads_ptr[i]->lado_calle){
                case 0:
                    calendarizacion_siguiente(LADO_IZQUIERDO, &hilo_actual_izquierda, &cola_de_listo_izquierda, &cola_de_listo_derecha);
                    break;
                case 1:
                    calendarizacion_siguiente(LADO_DERECHO, &hilo_actual_derecha, &cola_de_listo_izquierda, &cola_de_listo_derecha);
                    break;
                default:
                    printf("Ta malo esta vaina\n");
                    break;
            }   
        }
    }
    set_flag_hilo_actual_actualizado_CEthread(1);

    return 0;
}



//Funcion donde el thread actual cede voluntariamente la CPU. 
//Esto permite que otros hilos del mismo proceso o de otros procesos se ejecuten si están listos.
int CEthread_yield(){

    if (sched_yield() == -1) 
    {
        perror("CEthread_yield");
        exit(EXIT_FAILURE); 
    }
    return 0;
}


//Funcion para unir threads
int CEthread_join(CEthread_t* thread_ptr){

    //Si el thread no se puede unir, entonces se produce un error

    if(thread_ptr->state == DETACHED){
        perror("CEthread_join");
        exit(EXIT_FAILURE);
    }

    // Espera para que el thread cambie de estado a terminado
    // FINISHED
    while (thread_ptr->state != FINISHED){
        // Si no lo ha hecho, cede el paso a otro thread (a el mismo)
        //CEthread_yield();
        continue;
    } 

} 


int CEthread_detach(CEthread_t* thread_ptr){

    size_t pos = search_target_pid(thread_ptr->thread_id); //Buscar el thread con el pid para encontrar su posición en el arreglo 

    threads_ptr[pos]->state = DETACHED;  //Cambia de estado, lo que implica que ya no se puede hacer join() sobre ese hilo

    return 0;
}



/*
Busca un espacio libre en la matriz de hilos y establece el valor pid 
Parametro:
pid_t target: Identificador del nuevo proceso.
*/

void add_thread(pid_t target, CEthread_t** CEthread_ptr){

    // Manejar el error en caso de estar lleno 
    int isFound = FALSE;

    //Checar todos los espacios
    for (size_t i = 0; i < MAX_THREADS; i++)
    {
        // Si el espacio no esta utilizado llenarlo
        if(threads_pid[i] == -1){
            //actualizar
            isFound = TRUE; 
            // Crear un nuevo pid
            threads_pid[i] = target;
            // Crear un ptr para la gestion
            threads_ptr[i] = *CEthread_ptr;
            break;
        }

        // Si el espacio no se utiliza pero está lleno de basura
        else if(threads_pid[i] == -2){
            //actualizar
            isFound = TRUE; 
            // Crear un nuevo pid
            threads_pid[i] = target;
            // Free current thrash
            free(threads_ptr[i]);
            // Crear un ptr para la gestion
            threads_ptr[i] = *CEthread_ptr;

        }
    }
    
    //En caso de no lograr añadir el thread
    if(!isFound){
        perror("add_thread");
        exit(EXIT_FAILURE);
    }
    
}


/*
Esta función busca la posición de un pid objetivo en la matriz pid
 
Parámetros:
pid_t target: dentificador del proceso
 
Devuelve
size_t posición del objetivo en el array
 */

size_t search_target_pid(pid_t pid){

    // Indice del objetivo
    size_t i;
    short isFound = FALSE;

    for (i = 0; i < MAX_THREADS; i++)
    {
        // Si se encuentro el objetivo
        if(threads_pid[i] == pid){
    
            isFound = TRUE;
            // Detener el ciclo
            break;
        }
    }

    // En caso de no encontrarse 
    if(!isFound){
        perror("search_target_pid");
        exit(EXIT_FAILURE);
    }

    return i;

}

// Inicializa un mutex
int CEmutex_init(CEmutex_t* mutex) {
    if (!mutex) return -1;
    atomic_init(&mutex->locked, 0);
    mutex->owner = 0;
    return 0;
}

// Destruye un mutex
int CEmutex_destroy(CEmutex_t* mutex) {
    if (!mutex) {
        errno = EINVAL;
        return -1;  // Mutex inválido
    }
    
    // Verificar que el mutex no esté bloqueado
    if (atomic_load(&mutex->locked) != 0) {
        errno = EBUSY;
        return -1;  // Mutex aún está bloqueado
    }
    
    // Verificar que no haya hilos esperando
    if (mutex->owner != 0) {
        errno = EBUSY;
        return -1;  // Hay un owner asignado (indica uso reciente)
    }
    
    // Limpiar la estructura
    atomic_store(&mutex->locked, 0);
    mutex->owner = 0;
    
    return 0;  // Éxito
}

// Implementación mejorada de CEmutex_lock
int CEmutex_lock(CEmutex_t* mutex) {
    
    pid_t current = getpid();
    int expected = 0;
    
    // Intento atómico de adquirir el lock
    while (!atomic_compare_exchange_strong(&mutex->locked, &expected, 1)) {
        expected = 0;
        CEthread_yield();
    }
    
    mutex->owner = current;

    return 0;
}

// Implementación mejorada de CEmutex_unlock
int CEmutex_unlock(CEmutex_t* mutex) {
    /*
    if (mutex->owner != getpid()) return -1; // Solo el owner puede liberar
    */
    mutex->owner = 0;
    atomic_store(&mutex->locked, 0);
    
    return 0;
}

// Permite establecer la cola (izquierda o derecha) para el cambio de contexto, se debe indicar en cual cola se está dando el cambio de contexto
void set_lado_en_cambio_contexto_RR (short lado_calle){
    cambiar_lado_contexto_RR(lado_calle);
}

void set_detener_timer_RR(){
    detener_timer();
}

void set_reiniciar_timer_RR(){
    reiniciar_timer();
}

void set_algoritmo_calendarizacion_CEthread(short algoritmo){
    set_algoritmo_calendarizacion(algoritmo);
}

CEthread_queue_t* get_cola_izquierda(){
    return &cola_de_listo_izquierda;
}

CEthread_queue_t* get_cola_derecha(){
    return &cola_de_listo_derecha;
}

CEthread_t** get_hilo_actual_izquierda(){
    return &hilo_actual_izquierda;
}

CEthread_t** get_hilo_actual_derecha(){
    return &hilo_actual_derecha;
}

short* get_flag_hilo_actual_actualizado_CEthread(){
    return get_flag_hilo_actual_actualizado();
}

void set_flag_hilo_actual_actualizado_CEthread(short flag){
    set_flag_hilo_actual_actualizado(flag);
}

void set_quantum_CEthread(int valor_quantum){
    set_quantum(valor_quantum);
}

volatile short* get_flag_cambio_contexto_CEthread(){
    return get_flag_cambio_contexto();
}
void set_flag_cambio_contexto_CEthread(short flag_cambio_contexto){
    set_flag_cambio_contexto(flag_cambio_contexto);
}