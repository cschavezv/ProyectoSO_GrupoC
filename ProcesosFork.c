int main()
{
    struct timeval inicio, fin; // Para guardar el tiempo de inicio y fin

    // Guardamos el tiempo justo antes de comenzar el procesamiento
    gettimeofday(&inicio, NULL);

    // Creamos una memoria compartida para almacenar el contador de caras detectadas
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);

    // Verificamos si hubo error al crear la memoria compartida
    if (shmid == -1)
    {
        perror("Error creando memoria compartida");
        exit(1);
    }

    // Nos conectamos a la memoria compartida
    int *caras_detectadas = (int *)shmat(shmid, NULL, 0);

    // Inicializamos el contador en 0
    *caras_detectadas = 0;

    // Creamos un semáforo para controlar acceso a la memoria compartida
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("Error creando semáforo");
        // Limpiar memoria compartida antes de salir
        shmdt(caras_detectadas);
        shmctl(shmid, IPC_RMID, NULL);
        exit(1);
    }

    // Inicializamos el semáforo en 1 (disponible)
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("Error inicializando semáforo");
        shmdt(caras_detectadas);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        exit(1);
    }

    // Creamos un proceso hijo por cada cámara
    for (int i = 0; i < NUM_CAMARAS; i++)
    {
        pid_t pid = fork(); // Creamos el proceso

        if (pid == -1)
        {
            perror("Error en fork");
            // Limpieza antes de salir
            shmdt(caras_detectadas);
            shmctl(shmid, IPC_RMID, NULL);
            semctl(semid, 0, IPC_RMID);
            exit(1);
        }

        if (pid == 0)
        {
            // Este código solo se ejecuta en el proceso hijo
            procesarCamara(i, caras_detectadas, semid);

            // Nos desconectamos de la memoria compartida
            shmdt(caras_detectadas);

            // Terminamos el proceso hijo
            exit(0);
        }
    }

    // Este código lo ejecuta solo el proceso padre

    // Esperamos a que todos los procesos hijos terminen
    for (int i = 0; i < NUM_CAMARAS; i++)
    {
        wait(NULL);
    }

    // Guardamos el tiempo cuando ya terminó todo
    gettimeofday(&fin, NULL);

    // Calculamos cuánto tiempo pasó en milisegundos
    double tiempo_total = (fin.tv_sec - inicio.tv_sec) * 1000.0; // segundos a milisegundos
    tiempo_total += (fin.tv_usec - inicio.tv_usec) / 1000.0;     // microsegundos a milisegundos

    // Mostramos cuántas caras se detectaron entre todas las cámaras
    printf("\nTotal de caras detectadas: %d\n", *caras_detectadas);

    // Mostramos cuánto tiempo tomó todo el trabajo
    printf("Tiempo total con procesos: %.2f ms\n", tiempo_total);

    // Nos desconectamos de la memoria compartida
    shmdt(caras_detectadas);

    // Liberamos la memoria compartida
    shmctl(shmid, IPC_RMID, NULL);

    // Liberamos el semáforo
    semctl(semid, 0, IPC_RMID);

    return 0;
}