#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // para fork() y sleep
#include <sys/ipc.h> // para memoria compartida y semáforos
#include <sys/shm.h>
#include <sys/sem.h>  // para semáforos
#include <sys/wait.h> // para wait()
#include <time.h>     // para rand y time
#include <sys/time.h> // para gettimeofday()

#define NUM_CAMARAS 8
#define IMAGENES_POR_CAMARA 5

// Unión para semctl (System V semáforos)
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// Esta función simula el trabajo que hace una cámara cuando procesa una imagen.
// Simplemente muestra un mensaje para indicar qué cámara está trabajando y en qué imagen.
// Luego hace una pausa breve para simular que está usando la CPU.
void procesarImagen(int camara, int imagen)
{
    printf("Camara %d procesando imagen %d ...\n", camara, imagen);
    usleep(100000); // Pausa de 100 milisegundos para simular carga
}

// Esta función decide aleatoriamente si se detecta una cara en la imagen.
// Utiliza una probabilidad del 30% para simular detección exitosa.
int cara_detectada()
{
    // Se genera un número entre 0 y 9.
    // Si el número es menor que 3, se considera que se detectó una cara.
    return rand() % 10 < 3;
}

// Esta función simula una cámara de seguridad utilizando un proceso.
// Cada cámara procesa 5 imágenes y, si detecta una cara, incrementa el contador global.
// Este contador está en memoria compartida y protegido por un semáforo.
void procesarCamara(int camara, int *caras_detectadas, int semid)
{
    // Se inicializa una semilla única por proceso para evitar que todos generen los mismos números aleatorios
    srand(time(NULL) ^ getpid());

    // Definimos las operaciones para controlar el semáforo:
    struct sembuf p = {0, -1, 0}; // operación "wait" (bloquea)
    struct sembuf v = {0, 1, 0};  // operación "signal" (libera)

    // Cada cámara procesa 5 imágenes
    for (int i = 0; i < IMAGENES_POR_CAMARA; i++)
    {
        // Simulamos que la cámara está procesando la imagen
        procesarImagen(camara, i);

        // Verificamos si se detectó una cara
        if (cara_detectada())
        {
            // Antes de modificar el contador, se bloquea el semáforo para evitar condiciones de carrera
            semop(semid, &p, 1);

            // Se incrementa la variable compartida de caras detectadas
            (*caras_detectadas)++;

            // Se imprime información sobre la detección
            printf("Camara %d detectó cara en imagen %d (total: %d)\n", camara, i, *caras_detectadas);

            // Se libera el semáforo para que otros procesos puedan acceder a la variable compartida
            semop(semid, &v, 1);
        }
    }

    // Al finalizar el procesamiento de las imágenes, se indica que la cámara ha terminado.
    printf("Camara %d terminó procesamiento\n", camara);
}
