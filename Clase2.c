#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

#define BUFFER_SIZE 2048
#define END "FIN"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <ID de usuario>\nEjemplo: %s P1\n", argv[0], argv[0]);
        return 1;
    }

    char *usuario = argv[1];
    char fifo_lectura[64];
    char fifo_escritura[64];
    char buffer[BUFFER_SIZE];

    // Nombres FIFO
    snprintf(fifo_lectura, sizeof(fifo_lectura), "fifo_%s_in", usuario);
    snprintf(fifo_escritura, sizeof(fifo_escritura), "fifo_%s_out", usuario);

    // Crear FIFO de lectura si no existe
    if (mkfifo(fifo_lectura, 0666) == -1) {
        // Puede fallar si ya existe, no pasa nada
    }

    printf("Chat iniciado (%s)\nEscribe '%s' para salir.\n", usuario, END);

    // Abrir FIFO de lectura en modo no bloqueante
    int fd_lectura = open(fifo_lectura, O_RDONLY | O_NONBLOCK);
    if (fd_lectura == -1) {
        perror("Error al abrir fifo de lectura");
        return 1;
    }

    // Para enviar mensajes, abriremos el fifo de otros usuarios en cada envío
    // Por simplicidad aquí asumiremos comunicación directa (el usuario debe abrir fifo de otro usuario)

    fd_set fds;
    struct timeval tv;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(fd_lectura, &fds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(fd_lectura + 1, &fds, NULL, NULL, &tv);

        if (ret > 0) {
            if (FD_ISSET(fd_lectura, &fds)) {
                // Leer mensaje entrante
                ssize_t bytes = read(fd_lectura, buffer, BUFFER_SIZE - 1);
                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    printf("\nMensaje recibido: %s\n", buffer);
                    printf("[%s] Tu mensaje: ", usuario);
                    fflush(stdout);
                }
            }

            if (FD_ISSET(STDIN_FILENO, &fds)) {
                // Leer mensaje para enviar
                if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                    continue;
                }
                buffer[strcspn(buffer, "\n")] = 0;  // quitar salto de línea

                if (strcmp(buffer, END) == 0) {
                    break;
                }

                // Aquí debes decidir a qué usuario enviar (ejemplo: P1 envía a P2)
                // Para simplicidad pedimos el usuario destino en cada mensaje
                char destino[64];
                printf("Enviar a usuario (ejemplo P1, P2): ");
                fflush(stdout);
                if (fgets(destino, sizeof(destino), stdin) == NULL) {
                    continue;
                }
                destino[strcspn(destino, "\n")] = 0;

                // Abrir fifo destino para escribir
                char fifo_destino[64];
                snprintf(fifo_destino, sizeof(fifo_destino), "fifo_%s_in", destino);

                int fd_escritura = open(fifo_destino, O_WRONLY);
                if (fd_escritura == -1) {
                    printf("No se pudo abrir FIFO de %s. ¿Está el usuario conectado?\n", destino);
                    continue;
                }

                char mensaje_enviar[BUFFER_SIZE];
                snprintf(mensaje_enviar, sizeof(mensaje_enviar), "[%s] %s", usuario, buffer);
                write(fd_escritura, mensaje_enviar, strlen(mensaje_enviar));
                close(fd_escritura);

                printf("[%s] Tu mensaje: ", usuario);
                fflush(stdout);
            }
        }
    }

    close(fd_lectura);
    unlink(fifo_lectura);  // eliminar fifo al salir

    printf("\nChat finalizado para %s.\n", usuario);
    return 0;
}
