// chat_proceso.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 2032
#define TERMINAR1 "e"
#define TERMINAR2 "f"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <ID de usuario>\n", argv[0]);
        return 1;
    }

    char *usuario = argv[1];  // El nombre del proceso será pasado como argumento (P1, P2, P3, etc.)

    key_t key = ftok("shmfile", 65); // Asegúrate de crear este archivo
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    char *data = (char *)shmat(shmid, (void *)0, 0);
    
    char buffer[SHM_SIZE];
    char ultimo_mensaje[SHM_SIZE] = "";  // Para no mostrar el mismo mensaje varias veces

    while (1) {
        sleep(2);  // Espera para simular turnos

        // Leer lo que hay en la memoria
        if (strcmp(data, ultimo_mensaje) != 0) {
            printf("Mensaje recibido: %s\n", data);
            strcpy(ultimo_mensaje, data);
        }

        // Verifica si el mensaje es de salida
        if (strncmp(data, TERMINAR1, strlen(TERMINAR1)) == 0 ||
            strncmp(data, TERMINAR2, strlen(TERMINAR2)) == 0) {
            break;
        }

        printf("[%s] Tu mensaje (o ENTER para no escribir): ", usuario);
        fgets(buffer, SHM_SIZE, stdin);

        if (buffer[0] != '\n') {  // Si el usuario escribió algo
            buffer[strcspn(buffer, "\n")] = 0; // Eliminar salto de línea
            // Escribe el mensaje con el identificador
            // Aseguramos que el tamaño total no exceda SHM_SIZE
            snprintf(data, SHM_SIZE, "[%.*s] %.*s", SHM_SIZE-10, usuario, SHM_SIZE-10, buffer);

            // Terminar si el mensaje es de salida
            if (strcmp(buffer, TERMINAR1) == 0 || strcmp(buffer, TERMINAR2) == 0) {
                break;
            }
        }
    }

    shmdt(data);
    return 0;
}

