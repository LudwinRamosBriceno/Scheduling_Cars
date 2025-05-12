#include <gtk/gtk.h>
#include "CEThread.h"
#include "CEThread_utils.h"
#include <stdio.h>
#include <string.h>

int ancho_ventana = 900;
int alto_ventana = 700;
int altura_calle = 220;
GtkWidget *area;


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
    return 1; // éxito
}



gboolean dibujar(GtkWidget *widget, cairo_t *cr, gpointer data) {
    const Config *config = (const Config *)data;

    // Fondo blanco
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Calle negra centrada
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, (ancho_ventana - config->largo_calle) / 2, altura_calle, config->largo_calle, 170);
    cairo_fill(cr);

    return FALSE;
}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    static Config config;

    if (!leer_configuracion("archivo_config.txt", &config)) {
        return 1;
    }

    printf("largo_calle: %d\n", config.largo_calle);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduling Cars");
    gtk_window_set_default_size(GTK_WINDOW(window), ancho_ventana, alto_ventana);
    
    area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), area);

    // PASAR LA CONFIGURACIÓN A LA FUNCIÓN DRAW
    g_signal_connect(area, "draw", G_CALLBACK(dibujar), &config);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
