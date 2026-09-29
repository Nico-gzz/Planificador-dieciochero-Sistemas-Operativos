# Planificador Dieciochero

Simulador de actividades organizadas como un DAG (grafo acíclico dirigido), 
desarrollado para la Tarea 1 de Sistemas Operativos (UDP).

## Compilación

gcc -Wall -Wextra -std=c17 -o planificador main.c

## Uso

./planificador plan.txt K

Donde `plan.txt` es el archivo con las actividades y `K` es el número máximo 
de procesos que pueden ejecutarse simultáneamente.

## Formato de plan.txt

ID : Nombre : tiempo_ms : dep1, dep2, ...

Si el tiempo se deja vacío, se asigna aleatoriamente entre 100 y 5000 ms.

## Funciones implementadas

- **parsear_linea**: lee una línea de plan.txt y la convierte en una 
  estructura Actividad, separando ID, nombre, tiempo y dependencias.
- **trim**: elimina espacios en blanco al inicio y final de un texto, 
  necesario porque el formato de plan.txt incluye espacios alrededor 
  de los separadores.
- **buscar_indice**: busca la posición de una actividad en el arreglo 
  a partir de su ID (usado para resolver dependencias).
- **manejador_sigint**: capturador de la señal SIGINT (Ctrl+C). Al 
  recibirla, envía SIGTERM a todos los procesos hijos activos y 
  termina el programa.

## Decisiones de diseño

- **Control de concurrencia sin busy-waiting**: en vez de consultar 
  constantemente si hay procesos libres, el proceso padre usa wait() 
  de forma bloqueante, que solo retorna cuando un hijo realmente 
  termina. El límite K se respeta llevando un contador de procesos 
  activos.

- **Comunicación por pipes**: se crea un pipe independiente por cada 
  dependencia (arista del DAG) entre dos actividades, en vez de un 
  solo pipe por nodo. Esto evita que dos actividades que dependen de 
  un mismo nodo compitan por leer el mismo mensaje. Cada proceso 
  cierra los extremos de pipes que no le corresponden, tanto para 
  evitar fugas de descriptores de archivo como para que la lectura 
  detecte correctamente el fin de la escritura.

- **Aislamiento de errores**: cada actividad tiene un 10% de 
  probabilidad de fallar aleatoriamente (el enunciado no especifica 
  una causa de fallo concreta). Si una actividad falla, se marca como 
  tal y sus dependientes (directos e indirectos) se marcan como 
  abortados en cascada, sin afectar otras ramas del DAG que no 
  dependan de ella.

- **IDs como texto**: aunque el ejemplo del enunciado usa solo 
  números, la rúbrica especifica que el ID es "alfanumérico", por lo 
  que se optó por representarlo como texto (char[16]) en vez de 
  entero, para soportar ambos casos.

- **Límite de archivos abiertos**: con la prueba de estrés de 10000 
  actividades, la creación de pipes puede superar el límite por 
  defecto del sistema (1024 descriptores). El programa sube este 
  límite con setrlimit() al iniciar, para no depender de 
  configuración externa.

## Autor

Nicolas Gonzalez