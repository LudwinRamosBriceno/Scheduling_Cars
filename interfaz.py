import tkinter as tk
from PIL import Image, ImageTk

# Función para leer el archivo y obtener el valor de largo_calle
def obtener_datos(archivo):
    tiempo_letrero = None
    largo_calle = None
    carros_derecha = []
    carros_izquierda = []

    with open(archivo, "r") as f:
        for linea in f:
            if "tiempo_letrero" in linea:
                _, valor = linea.split(":")
                tiempo_letrero = int(valor.strip())
            elif "largo_calle" in linea:
                _, valor = linea.split(":")
                largo_calle = int(valor.strip())
            elif "carros_derecha" in linea:
                _, valor = linea.split(":")
                carros_derecha = list(map(int, valor.strip().split(",")))
            elif "carros_izquierda" in linea:
                _, valor = linea.split(":")
                carros_izquierda = list(map(int, valor.strip().split(",")))    
                

    return tiempo_letrero, largo_calle,carros_derecha,carros_izquierda

# Leer el valor de largo_calle desde el archivo
tiempo_letrero, largo_calle, carros_derecha, carros_izquierda = obtener_datos("archivo_config.txt")


# Si no se encuentra el valor, usamos un valor predeterminado
if largo_calle is None:
    largo_calle = 0  # Valor predeterminado


def flechas(canvas, tiempo_letrero):
    # Cargar imágenes
    img1 = ImageTk.PhotoImage(Image.open("img/derecha.png").resize((110, 110)))
    img2 = ImageTk.PhotoImage(Image.open("img/izquierda.png").resize((110, 110)))

    # Mostrar la primera imagen en el canvas (400 ancho)
    imagen_canvas = canvas.create_image(400, 140, anchor="nw", image=img1)

    # Guardar referencias para evitar recolección de basura
    canvas.img1 = img1
    canvas.img2 = img2


    # Función interna para cambiar la imagen
    def cambiar():
        # Cambiar entre las dos imágenes
        current_image = canvas.itemcget(imagen_canvas, "image")
        if current_image == str(canvas.img1):
            canvas.itemconfig(imagen_canvas, image=canvas.img2)
        else:
            canvas.itemconfig(imagen_canvas, image=canvas.img1)

        # Volver a programar el cambio de imagen
        canvas.after(tiempo_letrero, cambiar)  # Repetir la función después del delay

    # Llamamos a la función cambiar
    cambiar()


def cargar_imagenes_carros():
    return {
        0: ImageTk.PhotoImage(Image.open("img/normal1.png").resize((90, 90))),
        1: ImageTk.PhotoImage(Image.open("img/deportivo1.png").resize((90, 90))),
        2: ImageTk.PhotoImage(Image.open("img/emergencia1.png").resize((90,90)))
    }


def dibujar_carros(canvas, lista_carros, imagenes, extremo, x_calle, y_calle, largo_calle):
    espacio = 60  # Separación vertical entre carros
    canvas.update_idletasks()

    # X según el extremo
    if extremo == "izquierdo":
        x_pos = x_calle - 60  # Un poco a la izquierda del inicio de la calle
    else:  # "derecho"
        x_pos = x_calle + largo_calle + 60  # Un poco a la derecha del final de la calle

    for i, tipo in enumerate(lista_carros):
        imagen = imagenes.get(tipo)
        if not imagen:
            continue

        # Alinear los carros desde el borde superior de la calle hacia abajo
        y = y_calle + i * espacio
        canvas.create_image(x_pos, y, image=imagen, anchor="center")

        canvas.img_refs.append(imagen)



# Configuración del canvas
canvas_width = 900
canvas_height = 700

# Crear ventana
ventana_principal = tk.Tk()
ventana_principal.title("Scheduling Cars")
ventana_principal.geometry(f"{canvas_width}x{canvas_height}")

# Crear canvas
canvas = tk.Canvas(ventana_principal, width=canvas_width, height=canvas_height, bg="lightgray")
canvas.pack()

# Alto fijo (puedes hacerlo dinámico también si quieres)
alto = 200

# Calcular coordenadas para centrar el rectángulo
x_calle = (canvas_width - largo_calle) / 2
y_calle = (canvas_height - alto) / 2

# Dibujar el rectángulo centrado
canvas.create_rectangle(x_calle, y_calle, x_calle + largo_calle, y_calle + alto, fill="black", outline="gray", width=2)

# Dibujar una línea horizontal discontinua en el centro del rectángulo
y_centro = y_calle + alto / 2  # Coordenada y en el centro del rectángulo
canvas.create_line(x_calle, y_centro, x_calle + largo_calle, y_centro, fill="white", width=3, dash=(10, 10))  # Guion con patrón discontinuo

# Llamar a la función flechas para mostrar y cambiar las imágenes
flechas(canvas, tiempo_letrero)


# Leer datos
imagenes_carros = cargar_imagenes_carros()

# Guardar referencias de imágenes
canvas.img_refs = []

# Dibujar los carros en cada extremo
dibujar_carros(canvas, carros_izquierda ,imagenes_carros, "izquierdo",x_calle, y_calle, largo_calle)
dibujar_carros(canvas, carros_derecha, imagenes_carros, "derecho",x_calle, y_calle, largo_calle)

# Mostrar ventana
ventana_principal.mainloop()
