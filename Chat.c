// chat_proceso.c
// Chat con memoria compartida (tres procesos comunicándose entre terminales)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 2048
#define END "FIN"   // palabra para terminar el chat

int id_ya_en_uso(const char *data, const char *usuario) {
    // Si la memoria contiene el identificador, ya está en uso
    if (strstr(data, usuario) != NULL) {
        return 1; // ya existe
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <ID de usuario>\n", argv[0]);
        printf("Ejemplo: ./chat.out P1\n");
        return 1;
    }

    char *usuario = argv[1];  // Ejemplo: P1, P2, P3
    key_t key = ftok("shmfile", 65);

    if (key == -1) {
        perror("Error al crear la clave");
        return 1;
    }

    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Error al crear memoria compartida");
        return 1;
    }

    char *data = (char *)shmat(shmid, (void *)0, 0);
    if (data == (char *)(-1)) {
        perror("Error al adjuntar memoria compartida");
        return 1;
    }

    // --- NUEVA SECCIÓN: validar que el ID no se repita ---
    if (id_ya_en_uso(data, usuario)) {
        printf("❌ Error: el ID '%s' ya está en uso por otro proceso.\n", usuario);
        shmdt(data);
        return 1;
    }

    char registro[64];
    snprintf(registro, sizeof(registro), "%s conectado.", usuario);
    strcat(data, registro);
    strcat(data, "\n");

    char buffer[SHM_SIZE];
    char ultimo_mensaje[SHM_SIZE] = "";

    printf("=== Chat iniciado (%s) ===\n", usuario);
    printf("Escribe '%s' para salir.\n", END);

    while (1) {
        sleep(1);  // pequeña pausa para evitar saturación

        // Mostrar nuevos mensajes (excepto los propios)
        if (strcmp(data, ultimo_mensaje) != 0 && strlen(data) > 0) {
            if (strncmp(data, "[", 1) == 0 && strstr(data, usuario) == data + 1) {
                // El mensaje fue enviado por este mismo proceso → no mostrar
            } else {
                printf("\n%s\n", data);
                printf("[%s] Tu mensaje: ", usuario);
                fflush(stdout);
            }
            strcpy(ultimo_mensaje, data);
        }

        // Verificar si se envió la palabra de salida
        if (strncmp(data, END, strlen(END)) == 0) {
            break;
        }

        // Verificar si el usuario quiere escribir algo
        fd_set fds;
        struct timeval tv = {0, 100000};  // 0.1 segundos
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        int ready = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ready > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            fgets(buffer, SHM_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0; // quitar salto de línea

            if (strlen(buffer) > 0) {
                snprintf(data, SHM_SIZE, "[%s] %s", usuario, buffer);
                if (strcmp(buffer, END) == 0) {
                    break;
                }
            }
            printf("[%s] Tu mensaje: ", usuario);
            fflush(stdout);
        }
    }

    // Desconectar memoria
    shmdt(data);
