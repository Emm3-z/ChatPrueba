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
        snprintf(mensaje, sizeof(mensaje), "[%s] %
