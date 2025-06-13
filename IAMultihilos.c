#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define NUM_CAMARAS 8 //Cámaras de la A hasta la H, cada cámara va a representar a un hilo
#define IMAGENES_POR_CAMARAS 5 //Cada hilo/cámara tratará de detectar 5 caras

int caras_detectadas = 0; //Contador para todas las caras detectadas por la camara
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //iniciamos la variable mutex (herramienta de sincronización)
int detecciones[NUM_CAMARAS][IMAGENES_POR_CAMARAS]; //Creamos una matriz de detecciones

void procesarImagen(int camara, int imagen){
    /*Se mostrará que la cámara está procesando una imagen, esto se realiza para poder ver el flujo del trabajo*/
    printf("Camara %d procesando imagen %d ...\n", camara + 1, imagen); // Se suma 1 para mostrar cámaras del 1 al 8

    /*Se hará una pausa de 100 ms (Milisegundos) para simular el uso del CPU.
    La pausa se escribirá en 100000 microsegundos que es igual a 100 ms.
    Se lo hace en microsegundos porque C solo tiene la función para sleep en segundos "sleep" y
    microsegundos "usleep", entonces si queremos trabajar en milisegundos, debemos usar microsegundos.*/
    usleep(100000); 
}

void* procesarCamara(void* arg){

    /*Se hace un casteo del argumento recibido al tipo de estructura que almacena la cámara.
    Esto permite acceder dentro del hilo de forma ordenada.*/
    int camara = *(int*)arg;

    /*Con un for se va a ir recorriendo todas las imágenes que debe procesar la cámara 
    (5 en total como se lo estableció en la variable global)*/
    for (int i = 0; i < IMAGENES_POR_CAMARAS; i++){

        //Se empieza por las simulaciones, primero se va a simular que la cámara está procesando la imagen actual
        procesarImagen(camara, i);

        //Luego de procesar la imagen se detecta de manera aleatoria si se detectó una cara con la ayuda de un "if"

        if(detecciones[camara][i]){ //Se consulta la matriz pre-generada
            
            /*Antes de que se modifique la variable (caras_detectadas), se necesita que un solo hilo a la vez acceda
            a ella, para evitar que varios hilos lleguen y modifiquen el valor de manera simultánea, ya que, eso genera
            errores. Por esa razón se bloquea el mutex.*/
            pthread_mutex_lock(&mutex);
            
            caras_detectadas++; //se incrementa el contador de las caras detectadas
            
            /*Se imprime un mensaje que indica cuál cámara detectó la cara, en qué imagen se detectó la cara y cuántas
            caras se han detectado en total*/
            printf("Camara %d detectó cara en imagen %d (total: %d)\n",camara,i,caras_detectadas);
            
            /*Una vez ya se modificó la variable, se desbloquea el mutex para que los otros hilos puedan acceder
            al contador*/
            pthread_mutex_unlock(&mutex);
        }
    }
    /*Finalmente, se retorna NULL porque la función debe devolver un puntero void*,
    para cumplir con lo que requiere el pthread_create*/
    return NULL;
}

int main(){
    
    //Semilla fija para generación de números aleatorios idénticos entre procesos e hilos
    srand(42); 

    //Se genera la misma lista de detecciones antes de iniciar los hilos
    for(int i = 0; i < NUM_CAMARAS; i++){ //Bloque for que recorre la matriz
        for(int j = 0; j < IMAGENES_POR_CAMARAS; j++){
            detecciones[i][j] = (rand() % 10 < 3) ? 1 : 0; //Genera los mismos números randómicos entre los dos programas
        }
    }
    
    pthread_t hilos[NUM_CAMARAS]; //Se crea un arreglo de punteros identificadores del hilos
    int camaras_ids[NUM_CAMARAS]; //Se crea un arreglo de para asignar el número de cámara a usar (0-7)
    
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
        El cuarto parámetro (void *) es el parámetro que se le pasará a la función anterior cuando se ejecute el hilo aparte*/
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