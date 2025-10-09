/*
 * IPC usando archivos FIFO en Linux
 * Este programa muestra cómo varios procesos pueden comunicarse
 * usando un FIFO común (named pipe).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#define FIFO_PATH "./myfifo"
#define BUFFER_SIZE 2048
#define END "FIN"

// Función para escribir mensajes en FIFO
void writer(const char *usuario) {
    int fd;
    char buffer[BUFFER_SIZE];

    // Crear FIFO si no existe (modo 0666 para permitir lectura y escritura)
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        // Puede fallar si ya existe, no pasa nada
    }

    // Abrir FIFO en modo escritura
    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir FIFO para escritura");
        exit(EXIT_FAILURE);
    }

    printf("=== Chat iniciado (%s) ===\nEscribe '%s' para salir.\n", usuario, END);

    while (1) {
        printf("[%s] Tu mensaje: ", usuario);
        fflush(stdout);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            continue;
        }
        buffer[strcspn(buffer, "\n")] = 0; // quitar salto de línea

        if (strcmp(buffer, END) == 0) {
            // Enviar mensaje de fin para indicar cierre
            char msg_fin[BUFFER_SIZE];
            snprintf(msg_fin, sizeof(msg_fin), "[%s] %s", usuario, END);
            write(fd, msg_fin, strlen(msg_fin) + 1);
            break;
        }

        char mensaje[BUFFER_SIZE];
        snprintf(mensaje, sizeof(mensaje), "[%s] %s", usuario, buffer);
        write(fd, mensaje, strlen(mensaje) + 1);
    }

    close(fd);
}

// Función para leer mensajes desde FIFO
void reader(const char *usuario) {
    int fd;
    char buffer[BUFFER_SIZE];
    char ultimo_mensaje[BUFFER_SIZE] = "";

    // Crear FIFO si no existe
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        // Puede fallar si ya existe, no pasa nada
    }

    // Abrir FIFO en modo lectura no bloqueante
    fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("Error al abrir FIFO para lectura");
        exit(EXIT_FAILURE);
    }

    printf("=== Escuchando mensajes (%s) ===\n", usuario);

    while (1) {
        ssize_t bytes = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';

            // No mostrar mensajes propios
            if (strstr(buffer, usuario) != NULL) {
                // Es mensaje propio, ignorar
            } else if (strcmp(buffer + strlen(buffer) - strlen(END), END) == 0) {
                // Si es mensaje de fin, salir
                printf("\nChat terminado por otro usuario.\n");
                break;
            } else if (strcmp(buffer, ultimo_mensaje) != 0) {
                printf("\n%s\n", buffer);
                printf("[%s] Tu mensaje: ", usuario);
                fflush(stdout);
                strcpy(ultimo_mensaje, buffer);
            }
        }
        usleep(100000); // Espera 0.1 seg para evitar alta CPU
    }

    close(fd);
    unlink(FIFO_PATH); // Eliminar FIFO al terminar el chat
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s [writer|reader] <ID usuario>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *modo = argv[1];
    const char *usuario = argv[2];

    if (strcmp(modo, "writer") == 0) {
        writer(usuario);
    } else if (strcmp(modo, "reader") == 0) {
        reader(usuario);
    } else {
        fprintf(stderr, "Opción inválida. Usa 'writer' o 'reader'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
