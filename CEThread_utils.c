#include "CEThread_utils.h"


/*
 Gestiona los errores en la asignación de memoria
 
 Parámetros:
 size_t size: Tamaño en bytes del espacio de memoria asignado.

 Devuelve
 void* ptr a memoria
 */

void* allocate_mem(size_t size){

    void* mem = malloc(size);
    if (!mem) {
        perror("allocate_mem");
        exit(EXIT_FAILURE);
    }
    return mem;
}


 /*
 Utiliza la llamada al sistema clone para crear un hilo y gestiona los errores
 
 Parámetros:
 size_t size: Tamaño en bytes del espacio de memoria asignado
 
 Devuelve
 int pid al nuevo hilo.
 */

int clone_call(     int (*target_function) (void*), 
                    void* stack, 
                    int flags, 
                    void* args){

    // Lanza el hilo
    // Configurarlo para que se detenga y se inicie más tarde cuando la estructura esté lista
    int pid = clone(target_function, stack, flags, args);
    if (pid == -1) {
        perror("clone_call");
        exit(EXIT_FAILURE);
    }

    return pid;

}
