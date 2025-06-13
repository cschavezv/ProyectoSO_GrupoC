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
