import tkinter as tk
import random
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
                if valor:
                    carros_derecha = [int(v) for v in valor.split(",") if v.strip() != ""]
                else:
                    carros_derecha = []
            elif "carros_izquierda" in linea:
                _, valor = linea.split(":")
                valor = valor.strip()
                if valor:
                    carros_izquierda = [int(v) for v in valor.split(",") if v.strip() != ""]
                else:
                    carros_izquierda = []  
                

    return tiempo_letrero, largo_calle,carros_derecha,carros_izquierda



# Leer el valor de largo_calle desde el archivo
tiempo_letrero, largo_calle, carros_derecha, carros_izquierda = obtener_datos("archivo_config.txt")


# En caso de no encontrar valores en el archivo_config
if largo_calle is None:
    largo_calle = 0  # Valor predeterminado
if tiempo_letrero is None:
    tiempo_letrero = 0  # Valor predeterminado    


def flechas(canvas, tiempo_letrero):
    # Cargar imágenes
    img1 = ImageTk.PhotoImage(Image.open("img/derecha.png").resize((110, 110)))
    img2 = ImageTk.PhotoImage(Image.open("img/izquierda.png").resize((110, 110)))

    # Mostrar la primera imagen en el canvas (400 ancho, alto 155)
    imagen_canvas = canvas.create_image(400, 110, anchor="nw", image=img1)

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
        "izquierdo": {
            0: ImageTk.PhotoImage(Image.open("img/normal2.png").resize((80, 80))),
            1: ImageTk.PhotoImage(Image.open("img/deportivo2.png").resize((80, 80))),
            2: [
                
                ImageTk.PhotoImage(Image.open("img/bomberos2.png").resize((75,75))),
                ImageTk.PhotoImage(Image.open("img/ambulancia2.png").resize((80,80))),
                ImageTk.PhotoImage(Image.open("img/patrulla2.png").resize((80,80)))

            ]
        },
        "derecho": {
            0: ImageTk.PhotoImage(Image.open("img/normal1.png").resize((80, 80))),
            1: ImageTk.PhotoImage(Image.open("img/deportivo1.png").resize((80, 80))),
            2: [
                
                ImageTk.PhotoImage(Image.open("img/bomberos1.png").resize((75,75))),
                ImageTk.PhotoImage(Image.open("img/ambulancia1.png").resize((80,80))),
                ImageTk.PhotoImage(Image.open("img/patrulla1.png").resize((80,80)))
            ]
        }
    }


def dibujar_carros(canvas, lista_carros, imagenes, extremo, x_calle, y_calle, largo_calle, offset=0):
    espacio = 65  # Separación vertical entre carros
    canvas.update_idletasks()

    # X según el extremo
    if extremo == "izquierdo":
        x_pos = x_calle - 60
    else:  # "derecho"
        x_pos = x_calle + largo_calle + 60

    imagenes_extremo = imagenes.get(extremo, {})

    for i, tipo in enumerate(lista_carros):
        imagen = imagenes_extremo.get(tipo)
        if isinstance(imagen, list):
            imagen = random.choice(imagen)
        if not imagen:
            continue

        y = y_calle + (i + offset) * espacio
        canvas.create_image(x_pos, y, image=imagen, anchor="center")
        canvas.img_refs.append(imagen)



carros_colocados = {
    "izquierdo": len(carros_izquierda),
    "derecho": len(carros_derecha)
}


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

#Labels o textos

canvas.create_text(canvas_width/2, 70, text="Scheduling Cars", fill="black", font=("Courier New", 24, "bold"))
canvas.create_text(canvas_width/2, 430, text="Generar nuevo carro", fill="black", font=("Courier New", 16, "bold"))
canvas.create_text(canvas_width/2, 462, text="Coloque 0:normal, 1:deportivo, 2:emergencia", fill="black", font=("Courier New", 14, "bold"))
canvas.create_text(300, 495, text="Carril izquierdo", fill="black", font=("Courier New", 14, "bold"))
canvas.create_text(615, 495, text="Carril derecho", fill="black", font=("Courier New", 14, "bold"))


# Entry o campos de texto  (x,y)
entry_izq = tk.Entry(ventana_principal, font=("Arial", 14))
canvas.create_window(300, 525, window=entry_izq)
entry_der = tk.Entry(ventana_principal, font=("Arial", 14))
canvas.create_window(615, 525, window=entry_der)


def guardar_entry_izq():
    texto1 = entry_izq.get()
    if texto1.strip():  # Asegura que no esté vacío
        # Convierte a lista de enteros 
        carros_nuevos_izq = [int(x) for x in texto1.split(",") if x.strip().isdigit()]
        dibujar_carros(canvas, carros_nuevos_izq, imagenes_carros, "izquierdo", x_calle, y_calle, largo_calle, offset=carros_colocados["izquierdo"])
        carros_colocados["izquierdo"] += len(carros_nuevos_izq)

def guardar_entry_der():
    texto2 = entry_der.get()
    if texto2.strip():  # Asegura que no esté vacío
        # Convierte a lista de enteros 
        carros_nuevos_der = [int(x) for x in texto2.split(",") if x.strip().isdigit()]
        dibujar_carros(canvas, carros_nuevos_der, imagenes_carros, "derecho", x_calle, y_calle, largo_calle, offset=carros_colocados["derecho"])
        carros_colocados["derecho"] += len(carros_nuevos_der)


# Crear Botones
boton_izq = tk.Button(ventana_principal, text="Generar", bg= "white",command=guardar_entry_izq)
canvas.create_window(300, 560, window=boton_izq)
boton_der = tk.Button(ventana_principal, text="Generar", bg= "white",command=guardar_entry_der)
canvas.create_window(615,560, window=boton_der)


# Alto fijo (puedes hacerlo dinámico también si quieres)
alto = 170

# Calcular coordenadas para centrar el rectángulo
x_calle = (canvas_width - largo_calle) / 2
#y_calle = (canvas_height - alto) / 2
y_calle = 220

# Dibujar el rectángulo centrado
canvas.create_rectangle(x_calle, y_calle, x_calle + largo_calle, y_calle + alto, fill="black", outline="black", width=2)

# Dibujar una línea horizontal discontinua en el centro del rectángulo
y_centro = y_calle + alto / 2  # Coordenada y en el centro del rectángulo
canvas.create_line(x_calle, y_centro, x_calle + largo_calle, y_centro, fill="white", width=5, dash=(10, 10))  # Guion con patrón discontinuo

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
