#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s cantidad\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    srand(time(NULL));

    FILE *f = fopen("plan_grande.txt", "w");
    if (f == NULL) {
        fprintf(stderr, "No se pudo crear el archivo\n");
        return 1;
    }

    for (int i = 1; i <= n; i++) {
        fprintf(f, "%d : actividad_%d : %d : ", i, i, 100 + rand() % 400);

        int num_deps = 0;
        if (i > 1) {
            num_deps = rand() % 3;
            if (num_deps > i - 1) num_deps = i - 1;
        }

        for (int d = 0; d < num_deps; d++) {
            int dep = 1 + rand() % (i - 1);
            fprintf(f, "%d", dep);
            if (d < num_deps - 1) fprintf(f, ", ");
        }

        fprintf(f, "\n");
    }

    fclose(f);
    printf("Generado plan_grande.txt con %d actividades\n", n);
    return 0;
}