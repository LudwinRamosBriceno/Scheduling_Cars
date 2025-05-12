#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

int rect_length = 100; // Valor por defecto
int ancho_ventana = 900;
int alto_ventana = 700;
int carro_size = 30; // Tamaño del cuadrado
int altura_calle = 220;

// Función que busca "largo_calle:" y extrae su valor
void leer_desde_archivo() {
    FILE *archivo = fopen("archivo_config.txt", "r");
    if (archivo == NULL) {
        g_print("No se pudo abrir archivo_config.txt\n");
        return;
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        if (strncmp(linea, "largo_calle:", 12) == 0) {
            sscanf(linea + 12, "%d", &rect_length);
            break;
        }
    }

    fclose(archivo);
}

// Dibujar rectangulo de calle 
gboolean dibujar_calle(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 0, 0, 0); // Color negro
    cairo_rectangle(cr, (ancho_ventana - rect_length) / 2, altura_calle, rect_length, 170); // x, y, ancho (largo_calle), alto
    cairo_fill(cr);
    return FALSE;
}


// Callback de dibujo para un cuadrado
gboolean dibujar_carro(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 1, 0, 0); // Color negro
    cairo_rectangle(cr, 100, altura_calle, carro_size, carro_size); // x, y, tamaño del cuadrado
    cairo_fill(cr);
    return FALSE;
}


int main(int argc, char *argv[]) {
    
    gtk_init(&argc, &argv);

    leer_desde_archivo();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduling Cars");
    gtk_window_set_default_size(GTK_WINDOW(window), ancho_ventana, alto_ventana);

    GtkWidget *area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), area);

    g_signal_connect(area, "draw", G_CALLBACK(dibujar_calle), NULL);
     g_signal_connect(area, "draw", G_CALLBACK(dibujar_carro), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}







 
