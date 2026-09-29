#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "dag.h"

#define MAX_ACTIVIDADES 10000
#define MAX_EDGES 16

void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\n')) {
        s[--len] = '\0';
    }
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}

int parsear_linea(char *linea, Actividad *act) {
    char *token;

    token = strtok(linea, ":");
    if (token == NULL) return -1;
    strcpy(act->id, token);
    trim(act->id);

    token = strtok(NULL, ":");
    if (token == NULL) return -1;
    strcpy(act->nombre, token);
    trim(act->nombre);

    token = strtok(NULL, ":");
    if (token == NULL) return -1;
    if (strspn(token, " \t\n") == strlen(token)) {
        act->tiempo_ms = 100 + rand() % (5000 - 100 + 1);
    } else {
        act->tiempo_ms = atoi(token);
    }

    act->num_deps = 0;
    token = strtok(NULL, ",\n");
    while (token != NULL) {
        if (strspn(token, " \t") != strlen(token)) {
            sscanf(token, " %15s", act->deps[act->num_deps]);
            trim(act->deps[act->num_deps]);
            act->num_deps++;
        }
        token = strtok(NULL, ",\n");
    }

    return 0;
}

int buscar_indice(Actividad *actividades, int total, char *id) {
    for (int j = 0; j < total; j++) {
        if (strcmp(actividades[j].id, id) == 0) return j;
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s plan.txt K\n", argv[0]);
        return 1;
    }

    char *archivo = argv[1];
    int K = atoi(argv[2]);

    FILE *f = fopen(archivo, "r");
    if (f == NULL) {
        fprintf(stderr, "Error: no se pudo abrir %s\n", archivo);
        return 1;
    }

    static Actividad actividades[MAX_ACTIVIDADES];
    int total = 0;
    char linea[512];

    srand(time(NULL));

    while (fgets(linea, sizeof(linea), f) != NULL) {
        if (strspn(linea, " \t\n") == strlen(linea)) continue;
        if (parsear_linea(linea, &actividades[total]) == 0) {
            total++;
        }
    }

    fclose(f);

    printf("Se cargaron %d actividades (K=%d)\n", total, K);

    // Crear un pipe por cada dependencia (borde del DAG)
    static int pipe_read[MAX_ACTIVIDADES][MAX_EDGES];
    static int pipe_write_out[MAX_ACTIVIDADES][MAX_EDGES];
    static int num_out[MAX_ACTIVIDADES] = {0};

    for (int i = 0; i < total; i++) {
        for (int d = 0; d < actividades[i].num_deps; d++) {
            int idx_dep = buscar_indice(actividades, total, actividades[i].deps[d]);
            if (idx_dep == -1) continue;

            int fd[2];
            if (pipe(fd) == -1) {
                perror("pipe");
                return 1;
            }
            pipe_read[i][d] = fd[0];
            pipe_write_out[idx_dep][num_out[idx_dep]] = fd[1];
            num_out[idx_dep]++;
        }
    }

    int completado[MAX_ACTIVIDADES] = {0};
    int corriendo[MAX_ACTIVIDADES] = {0};
    pid_t pid_de[MAX_ACTIVIDADES];
    int terminados = 0;
    int activos = 0;

    while (terminados < total) {

        for (int i = 0; i < total; i++) {
            if (completado[i] || corriendo[i]) continue;
            if (activos >= K) break;

            int listo = 1;
            for (int d = 0; d < actividades[i].num_deps; d++) {
                int idx_dep = buscar_indice(actividades, total, actividades[i].deps[d]);
                if (idx_dep == -1 || !completado[idx_dep]) {
                    listo = 0;
                    break;
                }
            }

            if (!listo) continue;

            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Error en fork()\n");
                return 1;
            }

            if (pid == 0) {
                char buffer[128];
                for (int d = 0; d < actividades[i].num_deps; d++) {
                    ssize_t n = read(pipe_read[i][d], buffer, sizeof(buffer) - 1);
                    if (n > 0) {
                        buffer[n] = '\0';
                        printf("[hijo %d] recibido de %s: %s\n",
                               getpid(), actividades[i].deps[d], buffer);
                    }
                }

                printf("[hijo %d] ejecutando %s (%s), %dms\n",
                       getpid(), actividades[i].id, actividades[i].nombre,
                       actividades[i].tiempo_ms);

                struct timespec ts;
                ts.tv_sec = actividades[i].tiempo_ms / 1000;
                ts.tv_nsec = (actividades[i].tiempo_ms % 1000) * 1000000L;
                nanosleep(&ts, NULL);

                printf("[hijo %d] terminó %s\n", getpid(), actividades[i].id);

                char msg[128];
                snprintf(msg, sizeof(msg), "%s completada por PID %d", actividades[i].id, getpid());
                for (int k = 0; k < num_out[i]; k++) {
                    write(pipe_write_out[i][k], msg, strlen(msg) + 1);
                }

                exit(0);
            }

            corriendo[i] = 1;
            pid_de[i] = pid;
            activos++;
        }

        int status;
        pid_t pid_terminado = wait(&status);

        for (int i = 0; i < total; i++) {
            if (corriendo[i] && pid_de[i] == pid_terminado) {
                corriendo[i] = 0;
                completado[i] = 1;
                terminados++;
                activos--;
                break;
            }
        }
    }

    printf("Todas las actividades terminaron\n");
    return 0;
}