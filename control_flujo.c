#include "CEThread.h"
#include "control_flujo.h"
#include <signal.h>
#include <stdio.h>

CEthread_t** hilo_actual_izquierda_flujo = NULL; // Referencia al hilo actualmente en ejecución en la cola de izquierda
CEthread_t** hilo_actual_derecha_flujo = NULL;         // Referencia al hilo actualmente en ejecución en la cola de derecha
CEthread_queue_t* cola_de_listo_izquierda_flujo = NULL;  // referencia a la cola izquierda del calendarizador 
CEthread_queue_t* cola_de_listo_derecha_flujo = NULL;    // referencia a la cola derecha del calendarizador
short* flag_hilo_actual_actualizado_flujo = NULL;
volatile short* flag_cambio_contexto_RR_flujo = NULL;


void inicializar_parametros_flujo(CEthread_t** hilo_actual_izquierda, CEthread_t** hilo_actual_derecha,
                                CEthread_queue_t* cola_listo_izquierda, CEthread_queue_t* cola_listo_derecha){
    hilo_actual_izquierda_flujo = hilo_actual_izquierda;
    hilo_actual_derecha_flujo = hilo_actual_derecha;
    cola_de_listo_izquierda_flujo = cola_listo_izquierda;
    cola_de_listo_derecha_flujo = cola_listo_derecha;
    flag_hilo_actual_actualizado_flujo = get_flag_hilo_actual_actualizado_CEthread();
    flag_cambio_contexto_RR_flujo = get_flag_cambio_contexto_CEthread();

}


void control_flujo(short param_W, short tiempoLetrero, TipoFlujo algoritmoFlujo, Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion){
    switch (algoritmoFlujo) {
        case FLUJO_EQUIDAD:
            equidad(param_W, algoritmo_calendarizacion);
            break;
        case FLUJO_LETRERO:
            letrero(tiempoLetrero);
            break;
        default:
            equidad(param_W, algoritmo_calendarizacion);
    }
}


////////////////////////////////////////////////////// ALGORITMO DE EQUIDAD ////////////////////////////////////////////


void equidad(short param_W, Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion){
    short contador_cola1 = 0;
    short contador_cola2 = 0;

    //while(cola_de_listo_izquierda_flujo->count != 0 || cola_de_listo_derecha_flujo->count != 0){
    while((*hilo_actual_izquierda_flujo) != NULL || (*hilo_actual_derecha_flujo) != NULL){
        
        // Entra si es el turno de los carros de la cola de la izquierda
        if ((*hilo_actual_izquierda_flujo) != NULL && contador_cola1 < param_W){
            equidad_aux(hilo_actual_izquierda_flujo, algoritmo_calendarizacion);
            contador_cola1++;
        }
         // Entra si es el turno de los carros de la cola de la derecha
        else if ((*hilo_actual_derecha_flujo) != NULL && contador_cola2 < param_W){
            equidad_aux(hilo_actual_derecha_flujo, algoritmo_calendarizacion);
            contador_cola2++;
        }
        else{
            // Si las dos colas ya pasaron W carros cada una, se reinician los contadores para repetir el bucle
            contador_cola1 = 0;
            contador_cola2 = 0;            
        }
    }
}

void equidad_aux(CEthread_t** hilo_actual, Algoritmos_calendarizacion_en_flujo algoritmo_calendarizacion){

    (*hilo_actual)->state = RUNNING;  // se actualiza el estado del hilo en ejecutandose
    
    // se guarda una referencia al hilo que se está ejecutando, para cuando termine, saber su estado en el while sin que se produzca un error, pues hilo_actual cambia
    CEthread_t* hilo_actual_temp = *hilo_actual; 

    // Si el algoritmo es RR, entonces cada vez que se coloca a ejeucutar un hilo se reinicia el quantum
    if (algoritmo_calendarizacion == ROUND_ROBIN){
        set_lado_en_cambio_contexto_RR((*hilo_actual)->lado_calle);  // se coloca en qué cola se realiza el cambio de contexto
        set_flag_cambio_contexto_CEthread(0);
        set_reiniciar_timer_RR();
    }

    kill((*hilo_actual)->thread_id, SIGCONT); // pone a ejecutar el hilo

    // Se sale hasta que halla un cambio de contexto o que el hilo haya finalizado
    while (*flag_cambio_contexto_RR_flujo != 1 && hilo_actual_temp->state != FINISHED){
        usleep(1000);
    }
    
    // Acción preventiva para asegurar que el proceso principal (este), solo continue cuando se haya terminado por completo el hilo que se estaba ejecutando y evitar lecturas no deseadas
    while (*flag_hilo_actual_actualizado_flujo != 1){
        usleep(1000);
    }

    // Se baja la  bandera para la siguiente ejecución
    set_flag_hilo_actual_actualizado_CEthread(0);
}   



////////////////////////////////////////////////////// ALGORITMO DE LETRERO ////////////////////////////////////////////


void letrero(short tiempoLetrero){



}