int main(){
    pthread_t hilos[NUM_CAMARAS]; //Se crea un arreglo de punteros identificadores del hilos 
    int camaras_ids[NUM_CAMARAS]; //Se crea un arreglo que guardará el número de cámara que el hilo está simulando
    struct timespec inicio, fin; //Se declaran variables para medir el tiempo de ejecución del programa
    /*struct es una forma de agrupar varios datos relacionados en una sola variable. Es como un tipo de dato con distintos tipos de datos
    adentro. En este caso usaremos dos tipos de datos en el inicio y en el fin, para representar segundos y nanosegundos.
    timespec es una estructura (struct) para representar valores de tiempo con alta precisión -> tv_sec (segundos) y tv_nsec(nanosegundos)*/

    clock_gettime(CLOCK_MONOTONIC, &inicio); //Se usa la función clock_gettime para obtener el tiempo de ejecución
    /*En este caso es necesario entender que la función empezará desde aquí, ingresando los datos en la structura inicio. CLOCK_MONOTONIC
    es el tipo de reloj que se está usando. Se usa MONOTONIC en vez de REALTIME para evitar errores debido a que REALTIME toma el tiempo
    real del computador, es decir, si se cambia la hora, los resultados se verán afectados. MONOTONIC empieza a contar desde que que se lo
    llama, sin depender de la hora del computador*/

    //Crea un hilo por cámara, con su id
    for (int i = 0; i < NUM_CAMARAS; i++){ //bloque for para recorrer los arreglos hilos y camaras_ids
        camaras_ids[i] = i; //Asignamos a cada espacio un número (del 0 al 7) para que sea el número de cámara a simular
        pthread_create(&hilos[i], NULL, procesarCamara, &camaras_ids[i]);
        /*En el primer parámetro (pthread_t) se manda el puntero identificador del hilo correspondiente con i
        El segundo parámetro (pthread_attr_t) es el atributo de creación del hilo, mandamos NULL para que cree un hilo con atributos por defecto
        El tercer parámetro (void *(*)(void *)) es la función que se ejecutará como un hilo aparte, el hilo termina cuando la función lo hace
        El cuarto parámetro (void *) es el parámetro que se le pasará a la función anterior cuando se ejecute el hilo aparte
        No se usa solo &i en vez del arreglo camaras_ids porque los hilos pueden empezarse a ejecutar más tarde de lo que i aumenta*/
    }

    //Esperar a que todos los hilos terminen antes de continuar
    for (int i = 0; i < NUM_CAMARAS; i++){ //bloque for para recorrer el arreglo hilos
        pthread_join(hilos[i], NULL); //pthread_join permite que un hilo espere por otro
        /*El primer parámetro es el hilo i que se creó antes con pthread_create al cual quiero esperar*/
    }
    
    clock_gettime(CLOCK_MONOTONIC, &fin); //Toma la hora actual del CLOCK_MONOTONIC y lo guarda en la variable fin

    //Calcular cuánto tiempo pasó entre dos momentos (inicio y fin) dado en milisegundos
    double tiempo = (fin.tv_sec - inicio.tv_sec) * 1000.0; //Se restan los segundos completos entre inicio y fin, y lo convertimos a milisegundos
    tiempo += (fin.tv_nsec - inicio.tv_nsec) / 1000000.0; //Se restan los nanosegundos entre inicio y fin, y lo convertimos a milisegundos

    printf("\nTotal de caras detectadas: %d\n", caras_detectadas); //Imprimimos el total de caras detectadas
    printf("Tiempo total con hilos (con mutex): %.2f ms\n", tiempo); //Imprimimos el tiempo total que ha tardado todo el programa

    return 0;
}