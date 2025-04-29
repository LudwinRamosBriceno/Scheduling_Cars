#include <stdio.h>
#include <stdlib.h>
#include "CEThread.h"
#include <string.h>
#include <unistd.h> // para sleep


// Función que va a ejecutar el thread 
int print_mensaje(void* arg) {
    char* mensaje = (char*) arg;
    for (int i = 0; i < 5; ++i) {
        printf("Thread dice: %s\n", mensaje);
        sleep(1);
    }
    return 0;
}

int main() {
    printf("Programa principal iniciando...\n");

    CEthread_t* thread;           // Estructura del thread
    CEthread_attr_t* atributos;  // Atributos del thread

    char* mensaje = "Funcionando :)";
    
    pid_t tid = CEthread_create(&thread, &atributos, print_mensaje, mensaje);
    if (tid == -1) {
        printf("Error al crear el thread.\n");
        return 1;
    }

    // Esperar a que el thread termine
    CEthread_join(thread);
    
    printf("[DEBUG] Estado del hilo después de join: %d\n", thread->state);

    return 0;
}
