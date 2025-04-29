# Nombre del ejecutable
TARGET = programa

# Archivos fuente
SRCS = prueba.c CEThread.c CEThread_utils.c

# Archivos objeto (se obtienen reemplazando .c por .o)
OBJS = $(SRCS:.c=.o)

# Compilador y banderas
CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

# Regla por defecto
all: $(TARGET)

# Cómo construir el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Cómo compilar los .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f $(OBJS) $(TARGET)
