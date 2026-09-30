# Planificador Dieciochero

Simulador de actividades organizadas como un DAG (grafo acíclico dirigido), desarrollado para la Tarea 1 de Sistemas Operativos (UDP).

## Compilación

Para compilar en un entorno Unix, ejecute:
gcc -Wall -Wextra -std=c17 -o planificador main.c

## Uso

./planificador plan.txt K

Donde `plan.txt` es el archivo con las actividades y `K` es el límite de concurrencia máxima permitida.

## Formato del archivo plan.txt

ID : Nombre : tiempo_ms : dep1, dep2, ...
Si el tiempo se deja vacío, el simulador asigna dinámicamente un valor aleatorio entre 100 y 5000 ms.

## Funciones implementadas

- **parsear_linea**: lee una línea de plan.txt y la convierte en una estructura Actividad, separando ID, nombre, tiempo y dependencias. Se maneja el ID como cadena de texto alfanumérica para mayor flexibilidad.
- **trim**: elimina espacios en blanco al inicio y final de un texto, necesario para limpiar el formato del archivo de entrada.
- **buscar_indice**: busca la posición de una actividad en el arreglo a partir de su ID para mapear las dependencias del grafo.
- **manejador_sigint**: capturador de la señal SIGINT (Ctrl+C) configurado mediante la estructura `sigaction` (estandar de POSIX). Utiliza `kill(0, SIGTERM)` para enviar una señal de término de forma segura a todo el grupo de procesos activos sin iterar PIDs muertos.

## Decisiones de diseño y arquitectura

- **Control de concurrencia libre de busy-waiting**: Para limitar la concurrencia a `K` procesos simultáneos, el proceso padre orquesta las ejecuciones y utiliza `wait()` de forma estrictamente bloqueante. Esto detiene la ejecución del padre hasta que un hijo finaliza, cediendo la CPU y eliminando el desperdicio de ciclos asociado al busy-waiting.
- **Comunicación en cascada mediante pipes**: En lugar de compartir un único canal, se optó por crear tuberías (`pipe`) independientes para cada arista del DAG (entre actividad dependiente y su predecesora). Esto previene condiciones de carrera en la lectura. Además, cada proceso hijo cierra proactivamente los extremos de lectura/escritura que no utiliza, garantizando que no existan fugas de memoria y que la lectura finalice correctamente mediante `EOF`.
- **Aislamiento de fallos**: Si una actividad entra en fallo simulado (probabilidad del 10%), termina con un código de salida específico (`exit(2)`). El proceso padre intercepta este código mediante las macros `WIFEXITED` y `WEXITSTATUS`, marcando la rama dependiente como abortada sin comprometer el proceso principal ni la ejecución de ramas independientes.
- **Gestión de descriptores en pruebas de estrés (10.000 nodos)**: Debido a que la apertura de un pipe por cada arista escala linealmente, una carga de 10.000 actividades puede agotar el límite por defecto de descriptores de archivo del sistema (generalmente 1024). Para mitigar este cuello de botella y garantizar el éxito de la prueba de estrés, el programa invoca `setrlimit` al inicializar para solicitar al kernel la expansión dinámica del límite a 65536 *File Descriptors*.

## Autor
Nicolas Gonzalez
