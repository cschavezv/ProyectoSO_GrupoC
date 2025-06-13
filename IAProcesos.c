#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#define NUM_CAMARAS 8
#define IMAGENES_POR_CAMARA 5

// Unión necesaria para operaciones con semctl (System V semáforos)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// CAMBIO 1: Se usa una matriz de detección compartida por todos los procesos,
// para garantizar que todos lean los mismos valores aleatorios (como en la versión con hilos)
int detecciones[NUM_CAMARAS][IMAGENES_POR_CAMARA];

// Simula el procesamiento de una imagen por una cámara
void procesarImagen(int camara, int imagen){
    printf("Camara %d procesando imagen %d ...\n", camara, imagen);
    
}

// Simula el trabajo de una cámara que procesa imágenes y reporta detecciones de caras
void procesarCamara(int camara, int *caras_detectadas, int semid){
    // Operaciones para el semáforo: p (wait) y v (signal)
    struct sembuf p = {0, -1, 0};
    struct sembuf v = {0, 1, 0};

    // Cada cámara procesa 5 imágenes
    for (int i = 0; i < IMAGENES_POR_CAMARA; i++){
        procesarImagen(camara, i);

        // CAMBIO 2: Se consulta la matriz pre-generada en lugar de generar aleatoriamente
        if (detecciones[camara][i]){
            // Bloqueamos el semáforo antes de modificar la variable compartida
            semop(semid, &p, 1);

            (*caras_detectadas)++; // Se incrementa el contador global

            printf("Camara %d detectó cara en imagen %d (total: %d)\n", camara, i, *caras_detectadas);

            // Liberamos el semáforo
            semop(semid, &v, 1);
        }
    }

    // Mensaje de finalización por cada cámara
    printf("Camara %d terminó procesamiento\n", camara);
}

int main(){
    // CAMBIO 3: Semilla fija para asegurar mismos datos entre versión con procesos e hilos
    srand(42);

    // CAMBIO 4: Se pre-generan las detecciones para todas las cámaras e imágenes
    for(int i = 0; i < NUM_CAMARAS; i++){
        for(int j = 0; j < IMAGENES_POR_CAMARA; j++){
            // 30% de probabilidad de detectar una cara
            detecciones[i][j] = (rand() % 10 < 3) ? 1 : 0;
        }
    }

    struct timeval inicio, fin;
    gettimeofday(&inicio, NULL); // Se registra el tiempo inicial

    // Se crea una memoria compartida para el contador global
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1){ perror("shmget"); exit(1); }

    int *caras_detectadas = (int *)shmat(shmid, NULL, 0);
    *caras_detectadas = 0; // Inicializamos el contador

    // Creamos un semáforo para sincronizar el acceso a la variable compartida
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1){ perror("semget"); exit(1); }

    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg); // Inicializamos el semáforo en 1

    // Se crean procesos hijos para cada cámara
    for (int i = 0; i < NUM_CAMARAS; i++){
        pid_t pid = fork();
        if (pid == 0){
            // Cada hijo procesa su cámara correspondiente
            procesarCamara(i, caras_detectadas, semid);

            // El hijo se desconecta de la memoria compartida y termina
            shmdt(caras_detectadas);
            exit(0);
        }
    }

    // El proceso padre espera a que todos los hijos terminen
    for (int i = 0; i < NUM_CAMARAS; i++){
        wait(NULL);
    }

    gettimeofday(&fin, NULL); // Se registra el tiempo final

    // Se calcula el tiempo total en milisegundos
    double tiempo_total = (fin.tv_sec - inicio.tv_sec) * 1000.0;
    tiempo_total += (fin.tv_usec - inicio.tv_usec) / 1000.0;

    // Se imprime el total de caras detectadas y el tiempo de procesamiento
    printf("\nTotal de caras detectadas: %d\n", *caras_detectadas);
    printf("Tiempo total con procesos: %.2f ms\n", tiempo_total);

    // Limpieza de recursos compartidos
    shmdt(caras_detectadas);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
