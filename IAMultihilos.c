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

typedef struct { //La estructura almacena los parámetros que tendrá cada hilo
    int camara; //Identificador de la cámara
    unsigned int seed; //Semilla para generar números aleatorios dentro del hilo
} ParametrosHilo;

void procesarImagen(int camara, int imagen){
    /*Se mostrará que la cámara está procesando una imagen, esto se realiza para poder ver el flujo del trabajo*/
    printf("Camara %d procesando imagen %d ...\n", camara + 1, imagen); // Se suma 1 para mostrar cámaras del 1 al 8

    /*Se hará una pausa de 100 ms (Milisegundos) para simular el uso del CPU.
    La pausa se escribirá en 100000 microsegundos que es igual a 100 ms.
    Se lo hace en microsegundos porque C solo tiene la función para sleep en segundos "sleep" y
    microsegundos "usleep", entonces si queremos trabajar en milisegundos, debemos usar microsegundos.*/
    usleep(100000); 
}

int cara_detectada(unsigned int *seed){ //Mandamos como parámetro la semilla
    /*Va a retornar un número random entre 0 a 9 para simular que se encontró una cara. 
    Es decir si el número random es menor que 3 (0,1,2) se devolverá 1, señalando una detección facial exitosa.
    Por otro lado si no es menor que 3, se devolverá 0, indicando una detección facial fallida.*/
    return rand_r(seed) % 10 < 3; //Se usa rand_r que es una versión segura para hilos (thread-safe)
}

void* procesarCamara(void* arg){

    /*Se hace un casteo del argumento recibido al tipo de estructura que almacena la cámara y la semilla.
    Esto permite acceder a ambos valores dentro del hilo de forma ordenada. Se usa una estructura y no variables
    sueltas para evitar errores cuando se manejan muchos parámetros.*/
    ParametrosHilo *param = (ParametrosHilo *) arg;
    int camara = param->camara; //se usa -> cuando se tiene un puntero a una estructura y se quiere acceder a los campos internos
    unsigned int seed = param->seed;

    /*Con un for se va a ir recorriendo todas las imágenes que debe procesar la cámara 
    (5 en total como se lo estableció en la variable global)*/
    for (int i = 0; i < IMAGENES_POR_CAMARAS; i++){

        //Se empieza por las simulaciones, primero se va a simular que la cámara está procesando la imagen actual
        procesarImagen(camara, i);

        //Luego de procesar la imagen se detecta de manera aleatoria si se detectó una cara con la ayuda de un "if"
        if(cara_detectada(&seed)){ //Se pasa la dirección de la semilla para que rand_r funcione con datos únicos por hilo

            /*Antes de que se modifique la variable (caras_detectadas), se necesita que un solo hilo a la vez acceda
            a ella, para evitar que varios hilos lleguen y modifiquen el valor de manera simultánea, ya que, eso genera
            errores. Por esa razón se bloquea el mutex.*/
            pthread_mutex_lock(&mutex);

            //se incrementa el contador de las caras detectadas
            caras_detectadas++;

            /*Se imprime un mensaje que indica cuál cámara detectó la cara, en qué imagen se detectó la cara y cuántas
            caras se han detectado en total*/
            printf("Camara %d detectó cara en imagen %d (total: %d)\n", camara + 1, i, caras_detectadas);

            /*Una vez ya se modificó la variable, se desbloquea el mutex para que los otros hilos puedan acceder
            al contador*/
            pthread_mutex_unlock(&mutex);
        }
    }

    /*Se libera la memoria que fue asignada dinámicamente a la estructura.
    Esto es necesario porque se usó malloc al crear el hilo, y si no se libera,
    se produce una fuga de memoria.*/
    free(param);

    /*Finalmente, se retorna NULL porque la función debe devolver un puntero void*,
    para cumplir con lo que requiere el pthread_create*/
    return NULL;
}

int main(){
    pthread_t hilos[NUM_CAMARAS]; //Se crea un arreglo de punteros identificadores del hilos 
    ParametrosHilo parametros[NUM_CAMARAS]; //Se crea un arreglo de estructuras ParametrosHilo para poder asignar el id y la semilla
    //correspondiente a cada hilo, es decir, cada hilo tendrá su id de 0-7 (1 a 8 en pantalla) y su semilla exclusiva
    
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
        parametros[i].camara = i; //Asignamos a cada espacio un número (del 0 al 7) para que sea el número de cámara a simular
        parametros[i].seed = time(NULL) ^ (i + pthread_self()); //Generar una semilla única para cada hilo
        /*time(NULL) devuelve un número en segundos transcurridos desde el 1 de enero de 1970
        (i + pthread_self()) suma el índice i con el id único de cada hilo obtenido mediante pthread_self()
        Todo esto se concatena con un XOR para asegurarnos de que la semilla creada sea totalmente diferente en cada hilo*/
        pthread_create(&hilos[i], NULL, procesarCamara, &parametros[i]);
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