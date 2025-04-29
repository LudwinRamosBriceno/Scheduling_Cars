#ifndef CETHREAD_UTILS_H    //Evita inclusiones múltiples. Si otro archivo ya incluyó este .h, no lo volverá a procesar.
#define CETHREAD_UTILS_H

#define _GNU_SOURCE        //Habilita extensiones GNU en la libc, como clone()

#include <sched.h>     // Para clone()
#include <stdlib.h>    // Para malloc(), exit()
#include <stdio.h>     // Para perror()
#include <errno.h>     // Para códigos de error



#define FALSE 0
#define TRUE 1

// Estructura para pasar múltiples argumentos a la función clone()
// y empaqueta para llamar a lpthread_end cuando la ejecución haya finalizado

typedef struct CEthread_arguments
{
    int (*target_function) (void*);
    void* fnctn_args;
} CEthread_arguments_t;



void* allocate_mem(size_t size);

int clone_call(int (*target_function) (void*), 
               void* stack, 
               int flags, 
               void* args);


#endif 