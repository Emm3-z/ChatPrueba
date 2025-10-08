// chat_memoria.c
// Chat entre procesos con memoria compartida (ANSI C)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>

#define SHM_SIZE 2048
#define END "END"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <ID de usuario>\n", argv[0]);
        printf("Ejemplo: ./chat_memoria P1\n");
        return 1;
    }

    char *usuario = argv[1];

    // Crear o conectar al segmento de memoria compartida
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Error al crear memoria compartida");
        exit(1);
    }

    // Adjuntar la memoria al proceso actual
    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)(-1)) {
        perror("Error al adjuntar memoria compartida");
        exit(1);
    }

    printf("Proceso %s listo. Escribe mensajes (\"END\" para salir)\n", usuario);
    printf("------------------------------------------------------------\n");

    // Variables para manejar mensajes
    char buffer[SHM_SIZE];
    char ultimo_mensaje[SHM_SIZE] = "";

    // Configurar stdin sin buffer para que no espere Enter para leer mensajes nuevos
    setbuf(stdin, NULL);

    // Bucle principal de chat
    while (1) {
        // Revisar si hay un nuevo mensaje en memoria
        if (strlen(data) > 0 && strcmp(data, ultimo_mensaje) != 0) {
            printf("\n📩 %s\n> ", data);
            fflush(stdout);  // Forzar impresión inmediata
            strcpy(ultimo_mensaje, data);
        }

        // Revisar si el mensaje recibido fue "END"
        if (strncmp(data, END, strlen(END)) == 0)
            break;

        // Ver si el usuario escribió algo
        fd_set fds;
        struct timeval tv = {0, 200000}; // Espera 0.2 segundos
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        int ready = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ready > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            fgets(buffer, SHM_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0; // Quitar salto de línea

            // Escribir en la memoria compartida
            snprintf(data, SHM_SIZE, "[%s] %s", usuario, buffer);

            if (strcmp(buffer, END) == 0)
                break;

            printf("> ");
            fflush(stdout);
        }
    }

    // Desconectar la memoria compartida
    shmdt(data);

    // Si este proceso escribe "END", eliminar la memoria compartida
    if (strcmp(buffer, END) == 0) {
        printf("Finalizando chat...\n");
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}
