#include <gtk/gtk.h>
#include "CEThread.h"
#include "CEThread.c"
#include "CEThread_utils.c"
#include "CEThread_utils.h"
#include <stdio.h>
#include <string.h>

int ancho_ventana = 900;
int alto_ventana = 700;
int altura_calle = 220;
GtkWidget *area;
GtkWidget *entry;
GtkWidget *fixed;
CEthread_t hilo;


typedef struct {
    int tipo;   // 0 = rojo, 1 = azul, 2 = verde
    int x;
    int y;
} Carro;

#define MAX_CARROS 10

Carro carros_derecha[MAX_CARROS];
int num_carros_derecha_actual = 0;

Carro carros_izquierda[MAX_CARROS];
int num_carros_izquierda_actual = 0;


typedef struct {
    int tiempo_letrero;
    int cantidad_carros;
    int velocidad_carros;
    int largo_calle;
    char metodo_control_flujo[50];
    int carros_derecha[10];
    int num_carros_derecha;
    int carros_izquierda[10];
    int num_carros_izquierda;
} Config;


void parse_lista_carros(char *linea, int *array, int *cantidad) {
    *cantidad = 0;
    char *token = strtok(linea, ",");
    while (token != NULL && *cantidad < 10) {
        array[*cantidad] = atoi(token);
        (*cantidad)++;
        token = strtok(NULL, ",");
    }
}


int leer_configuracion(const char *nombre_archivo, Config *config) {
    FILE *archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        perror("Error abriendo archivo");
        return 0;
    }
    char linea[128];
    while (fgets(linea, sizeof(linea), archivo)) {
        char *clave = strtok(linea, ":");
        char *valor = strtok(NULL, "\n");

        if (!clave || !valor) continue;
        while (*valor == ' ') valor++; // eliminar espacios iniciales

        if (strcmp(clave, "tiempo_letrero") == 0)
            config->tiempo_letrero = atoi(valor);
        else if (strcmp(clave, "cantidad_carros") == 0)
            config->cantidad_carros = atoi(valor);
        else if (strcmp(clave, "velocidad_carros") == 0)
            config->velocidad_carros = atoi(valor);
        else if (strcmp(clave, "largo_calle") == 0)
            config->largo_calle = atoi(valor);
        else if (strcmp(clave, "metodo_control_flujo") == 0)
            strncpy(config->metodo_control_flujo, valor, sizeof(config->metodo_control_flujo) - 1);
        else if (strcmp(clave, "carros_derecha") == 0)
            parse_lista_carros(valor, config->carros_derecha, &config->num_carros_derecha);
        else if (strcmp(clave, "carros_izquierda") == 0)
            parse_lista_carros(valor, config->carros_izquierda, &config->num_carros_izquierda);
    }

    fclose(archivo);
    return 1; 
}



gboolean dibujar(GtkWidget *widget, cairo_t *cr, gpointer data) {
    const Config *config = (const Config *)data;

    // Fondo blanco
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Calle negra 
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, (ancho_ventana - config->largo_calle) / 2, altura_calle, config->largo_calle, 170);
    cairo_fill(cr);


    cairo_set_font_size(cr, 25);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,350, altura_calle-100);    // Mueve a la posición (10, 10)
    cairo_show_text(cr, "Scheduling Cars");  // Dibuja el texto


    cairo_set_font_size(cr, 22);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,325, altura_calle+220);    // Mueve a la posición (10, 10)
    cairo_show_text(cr, "Generar nuevo carro");  // Dibuja el texto

    cairo_set_font_size(cr, 20);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,220, altura_calle+255);    // Mueve a la posición (10, 10)
    cairo_show_text(cr, "Coloque 0:emergencia, 1:deportivo, 2:normal");  // Dibuja el texto

    cairo_set_font_size(cr, 20);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,220, altura_calle+300);    // Mueve a la posición (10, 10)
    cairo_show_text(cr, "Carril izquierdo");  // Dibuja el texto

    cairo_set_font_size(cr, 20);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,528, altura_calle+300);    // Mueve a la posición (10, 10)
    cairo_show_text(cr, "Carril derecho");  // Dibuja el texto


    for (int i = 0; i < num_carros_derecha_actual; i++) {
        switch (carros_derecha[i].tipo) {
            case 0: cairo_set_source_rgb(cr, 1, 0, 0); break;
             case 1: cairo_set_source_rgb(cr, 0, 0, 1); break;
             case 2: cairo_set_source_rgb(cr, 0, 1, 0); break;
             default: cairo_set_source_rgb(cr, 0, 0, 0); break;
        }
        cairo_rectangle(cr, carros_derecha[i].x, carros_derecha[i].y, 30, 20);
        cairo_fill(cr);
    }
    
    for (int i = 0; i < num_carros_izquierda_actual; i++) {
        switch (carros_izquierda[i].tipo) {
            case 0: cairo_set_source_rgb(cr, 1, 0, 0); break;
            case 1: cairo_set_source_rgb(cr, 0, 0, 1); break;
            case 2: cairo_set_source_rgb(cr, 0, 1, 0); break;
            default: cairo_set_source_rgb(cr, 0, 0, 0); break;
        }
        cairo_rectangle(cr, carros_izquierda[i].x, carros_izquierda[i].y, 30, 20);
        cairo_fill(cr);
   }

    return FALSE;
}


gboolean refrescar_area(gpointer data) {
    gtk_widget_queue_draw(area);
    return G_SOURCE_REMOVE;
}


int hilo_carro(void *arg) {
    g_idle_add(refrescar_area, NULL); 
    return NULL;
}



int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    static Config config;

    if (!leer_configuracion("archivo_config.txt", &config)) {
        return 1;
    }

    printf("largo_calle: %d\n", config.largo_calle);

    // DERECHA
    num_carros_derecha_actual = config.num_carros_derecha;
    for (int i = 0; i < num_carros_derecha_actual; i++) {
        carros_derecha[i].tipo = config.carros_derecha[i];
        carros_derecha[i].x = ancho_ventana - 130;  // desde la derecha
        carros_derecha[i].y = altura_calle + i * 30;  //para distanciar uno de otro
        CEthread_create(&hilo, NULL, hilo_carro, &carros_derecha[i]);
    }
    
    // IZQUIERDA
    num_carros_izquierda_actual = config.num_carros_izquierda;
    for (int i = 0; i < num_carros_izquierda_actual; i++) {
        carros_izquierda[i].tipo = config.carros_izquierda[i];
        carros_izquierda[i].x = 100;  // desde la izquierda
        carros_izquierda[i].y = altura_calle + i * 30;  // un poco más abajo
        CEthread_create(&hilo, NULL, hilo_carro, &carros_izquierda[i]);
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduling Cars");
    gtk_window_set_default_size(GTK_WINDOW(window), ancho_ventana, alto_ventana);


    // Crear el GtkEntry
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Escribe algo aquí...");


    area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), area);

    g_signal_connect(area, "draw", G_CALLBACK(dibujar), &config);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

