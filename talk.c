// chat_proceso.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 1024
#define TERMINAR1 "END"
#define TERMINAR2 "quit"

int main() {
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

        printf("Tu mensaje (o ENTER para no escribir): ");
        fgets(buffer, SHM_SIZE, stdin);

        if (buffer[0] != '\n') {  // Si el usuario escribió algo
            buffer[strcspn(buffer, "\n")] = 0; // Eliminar salto de línea
            strcpy(data, buffer);

            if (strcmp(buffer, TERMINAR1) == 0 || strcmp(buffer, TERMINAR2) == 0) {
                break;
            }
        }
    }

    shmdt(data);
    return 0;
}
