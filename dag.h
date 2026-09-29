#ifndef DAG_H
#define DAG_H

typedef struct {
    char id[16];
    char nombre[64];
    int tiempo_ms;
    char deps[16][16];
    int num_deps;
} Actividad;

#endif
