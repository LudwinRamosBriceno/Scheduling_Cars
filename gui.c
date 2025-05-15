#include <gtk/gtk.h>
#include "CEThread.h"
#include "CEThread.c"
#include "CEThread_utils.c"
#include "CEThread_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int ancho_ventana = 900;
int alto_ventana = 700;
int altura_calle = 220;
#define MAX_CARROS 10
GtkWidget *area;
GtkWidget *entry;
GtkWidget *entry2;
GtkWidget *fixed;
GtkWidget *label;
GtkWidget *boton;
GtkWidget *boton2;

CEthread_t hilo;
gboolean direccion = TRUE; // FALSE: "-->", TRUE: "<--"



typedef struct {
    int tipo;   // 0 = rojo, 1 = azul, 2 = verde
    int x;
    int y;
} Carro;


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


typedef struct {
    Carro *carro;
    Config *config;
} CarroArgs;


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
    cairo_move_to(cr,350, altura_calle-100);    
    cairo_show_text(cr, "Scheduling Cars");  


    cairo_set_font_size(cr, 22);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,325, altura_calle+220);   
    cairo_show_text(cr, "Generar nuevo carro");  

    cairo_set_font_size(cr, 20);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,220, altura_calle+255);    
    cairo_show_text(cr, "Coloque 0:emergencia, 1:deportivo, 2:normal");  

    cairo_set_font_size(cr, 20);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,220, altura_calle+300);    
    cairo_show_text(cr, "Carril izquierdo");  

    cairo_set_font_size(cr, 20);  // tamaño de fuente 
    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0, 0, 0);  // Color negro
    cairo_move_to(cr,528, altura_calle+300);    
    cairo_show_text(cr, "Carril derecho");  

    //Dibujar carros
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


gboolean letrero(gpointer user_data) {
    if (direccion) {
        gtk_label_set_text(GTK_LABEL(label), "-->");
        GdkRGBA azul;
        gdk_rgba_parse(&azul, "blue");
        gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &azul);
    } else {
        gtk_label_set_text(GTK_LABEL(label), "<--");
        GdkRGBA rojo;
        gdk_rgba_parse(&rojo, "red");
        gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &rojo);
    }

    direccion = !direccion;
    return TRUE;
}



gboolean refrescar_area(gpointer data) {
    gtk_widget_queue_draw(area);
    return G_SOURCE_REMOVE;
}


int hilo_carro(void *arg) {

    CarroArgs *args = (CarroArgs *)arg;
    Carro *carro = args->carro;
    Config *config = args->config;


    // Dirección de movimiento: derecha (-x) o izquierda (+x)
    int direccion = (carro->x > ancho_ventana / 2) ? -1 : 1;

    // Velocidad base (cuanto mayor, más rápido)
    switch (carro->tipo) {
        case 0: config->velocidad_carros = 5; break;  // Emergencia (rápido)
        case 1: config->velocidad_carros = 3; break;  // Deportivo (medio)
        case 2: config->velocidad_carros = 2; break;  // Normal (lento)
        default: config->velocidad_carros = 1;
    }

    for (int i = 0; i < 200; ++i) {  
        carro->x += direccion * config->velocidad_carros;
        g_idle_add(refrescar_area, NULL);
        usleep(10000); // milisegundos
    }
    g_idle_add(refrescar_area, NULL); 

    return NULL;
}



int crear_carro(Config *config){

    // DERECHA
    num_carros_derecha_actual = config->num_carros_derecha;
    for (int i = 0; i < num_carros_derecha_actual; i++) {
        carros_derecha[i].tipo = config->carros_derecha[i];
        carros_derecha[i].x = ancho_ventana - 130;  // desde la derecha
        carros_derecha[i].y = altura_calle + i * 30;  //para distanciar uno de otro
        CarroArgs *args = malloc(sizeof(CarroArgs));
        args->carro = &carros_derecha[i];
        args->config = config;
        //CEthread_create(&hilo, NULL, hilo_carro, &carros_derecha[i]);
        CEthread_create(&hilo, NULL, hilo_carro, args);

    }
    
    // IZQUIERDA
    num_carros_izquierda_actual = config->num_carros_izquierda;
    for (int i = 0; i < num_carros_izquierda_actual; i++) {
        carros_izquierda[i].tipo = config->carros_izquierda[i];
        carros_izquierda[i].x = 100;  // desde la izquierda
        carros_izquierda[i].y = altura_calle + i * 30;  // un poco más abajo
        CarroArgs *args = malloc(sizeof(CarroArgs));
        args->carro = &carros_izquierda[i];
        args->config = config;
        //CEthread_create(&hilo, NULL, hilo_carro, &carros_izquierda[i]);
        CEthread_create(&hilo, NULL, hilo_carro, args);
    }
}


void nuevos_izquierda(GtkButton *boton, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);       
    const gchar *texto = gtk_entry_get_text(entry);
    g_print("Texto ingresado: %s\n", texto);
}

void nuevos_derecha(GtkButton *boton2, gpointer user_data) {
    GtkEntry *entry2 = GTK_ENTRY(user_data);       
    const gchar *texto2 = gtk_entry_get_text(entry2);
    g_print("Texto ingresado: %s\n", texto2);
}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    static Config config;
    static Carro carro;

    if (!leer_configuracion("archivo_config.txt", &config)) {
        return 1;
    }

    printf("largo_calle: %d\n", config.largo_calle);


    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduling Cars");
    gtk_window_set_default_size(GTK_WINDOW(window), ancho_ventana, alto_ventana);

    //Crear fixed
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);
    
    
    // Área de dibujo
    area = gtk_drawing_area_new();
    gtk_widget_set_size_request(area, 900, 700);
    g_signal_connect(area, "draw", G_CALLBACK(dibujar), &config);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_fixed_put(GTK_FIXED(fixed), area, 0, 0);  


    // Etiqueta de letrero
    label = gtk_label_new("");
    gtk_widget_set_size_request(label, 50, 30);  //Tamaño
    gtk_fixed_put(GTK_FIXED(fixed), label,420, 140);  //Posicion
    g_timeout_add(1000, letrero, NULL);
    
    // Cambiar fuente y color (
    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, "DejaVu Serif");
    pango_font_description_set_size(desc, 30 * PANGO_SCALE); // Tamaño
    pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD); // Negrita
    gtk_widget_override_font(label, desc); // Aplicar fuente
    pango_font_description_free(desc);
    // Cambiar color a azul inicialmente
    GdkRGBA color;
    gdk_rgba_parse(&color, "blue");
    gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &color);


    // Entry 1 carril izquierdo
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "0,1,2..");
    gtk_widget_set_size_request(entry, 170, 30);  //Tamaño
    gtk_fixed_put(GTK_FIXED(fixed), entry, 220, 540);   //Posicion
    
    // Entry 2 carril derecho
    entry2 = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry2), "0,1,2..");
    gtk_widget_set_size_request(entry2, 170, 30);   //Tamaño
    gtk_fixed_put(GTK_FIXED(fixed), entry2, 528, 540);  //Posicion 


    //Boton izquierdo 
    GtkWidget *boton = gtk_button_new_with_label("Agregar");
    g_signal_connect(boton, "clicked", G_CALLBACK(nuevos_izquierda), entry);
    gtk_widget_set_size_request(boton, 100, 40);   //Tamaño
    gtk_fixed_put(GTK_FIXED(fixed), boton, 250, 580);  // X, Y


    //Boton izquierdo 
    GtkWidget *boton2 = gtk_button_new_with_label("Agregar");
    g_signal_connect(boton2, "clicked", G_CALLBACK(nuevos_derecha), entry2);
    gtk_widget_set_size_request(boton2, 100, 40);   //Tamaño
    gtk_fixed_put(GTK_FIXED(fixed), boton2, 568, 580);  // X, Y


    gtk_widget_show_all(window);

    crear_carro(&config);
    gtk_main();

    return 0;
}

