# Proyecto de Comparación de Desempeño: Procesos vs Hilos en Detección de Caras

## 📌 Descripción General

El propósito de este proyecto es comparar el rendimiento y el comportamiento entre dos métodos de concurrencia en C:

- **Procesos (fork + memoria compartida + semáforos System V)**
- **Hilos (pthreads + mutex)**

Ambos programas simulan un sistema de cámaras de vigilancia que procesan imágenes y detectan rostros. El sistema consta de:

- 8 cámaras (simuladas como hilos o procesos según el programa).
- Cada cámara procesa 5 imágenes.
- Cada imagen tiene un 30% de probabilidad de contener un rostro.

El código genera previamente la lista de detecciones para que ambos programas procesen exactamente los mismos datos, asegurando así una comparación justa del tiempo de ejecución.

Se miden los tiempos de ejecución de cada implementación para evaluar la eficiencia de los hilos frente a los procesos.

---

## 🧮 Operación del Programa

- Se genera una secuencia fija de detecciones mediante números aleatorios controlados por una semilla (`srand(42)`).
- Cada cámara procesa sus imágenes:
  - Simula un retardo de 100ms por imagen para representar carga de procesamiento.
  - Si se detecta una cara en la imagen, se incrementa un contador global protegido por sincronización:
    - **Procesos:** Semáforo System V.
    - **Hilos:** Mutex de pthreads.
- Al finalizar, se muestra:
  - El número total de caras detectadas.
  - El tiempo total de ejecución en milisegundos.

Este procedimiento permite comparar el **overhead de sincronización y concurrencia** de ambos mecanismos.

---

## 🚀 Procedimiento de Ejecución en Linux

A continuación, los pasos para compilar y ejecutar ambos programas:

### 1️⃣ Compilación

**Para el programa multihilos:**

gcc -pthread IAMultihilos.c -o IAMultihilos

**Para el programa con procesos:**

gcc -pthread IAProcesos.c -o IAProcesos

⚠ **Nota: La opción -pthread es necesaria para habilitar el soporte de threads en la biblioteca estándar de C.**

### 2️⃣ Generación del archivo ejecutable

Al compilar, se creará un archivo ejecutable:

IAMultihilos para la versión con hilos.

IAProcesos para la versión con procesos.

### 3️⃣ Ejecución

**Para ejecutar el programa multihilos:**

./IAMultihilos

**Para ejecutar el programa de procesos:**

./IAProcesos

### ⚙ Requisitos

- Sistema operativo Linux (distribución con soporte para GCC).

- Compilador gcc instalado.

- Permisos para ejecutar procesos e hilos.

### 📝 Autores

- Sebastián Chávez
- Frank Jumbo
- Josue Calvopiña
- Lady Velásquez

---

# 📓 Resultados

## Caso 1

![Caso1 (2)](https://github.com/user-attachments/assets/a2b386c6-9ee9-4f03-a84d-28ce53d39953)
![Caso1](https://github.com/user-attachments/assets/573c2757-a5f2-4da4-aecb-201e7bda8a4b)

- Este es el comportamiento más común y esperado, debido a que los hilos comparten el mismo espacio de memoria y su creación es menos costosa para el sistema operativo.
  
- La creación de hilos (pthread_create) es mucho más liviana que la creación de procesos (fork()), ya que no implica copiar todo el espacio de memoria.
  
- Los hilos comparten variables globales, por lo que no es necesario implementar mecanismos complejos de comunicación entre ellos.

- El acceso a datos compartidos es más eficiente siempre que haya un control adecuado de sincronización.
  
- En este caso, se utilizó un mutex (pthread_mutex_t) para proteger el contador global de caras detectadas, evitando condiciones de carrera.

## Caso 2

![Caso2 (2)](https://github.com/user-attachments/assets/c0174172-6160-470b-aab9-e92793c89056)
![Caso2](https://github.com/user-attachments/assets/3ea97eec-d26f-428d-8b4c-6a9740466cd2)

Aunque los hilos suelen ser más eficientes por compartir memoria y tener menos overhead, en algunas ejecuciones los procesos resultaron más rápidos. Esto puede deberse a:

- **Carga del sistema:** El planificador del SO puede dar más CPU a los procesos en ciertos momentos.

- **Planificación del CPU:** Aunque usan el mismo planificador, los procesos tienen memoria aislada, lo que a veces mejora su desempeño.

- **Sincronización ligera:** En nuestras pruebas, los procesos usaron semáforos pocas veces, reduciendo su overhead.

- **Memoria y caché:** Los procesos, al tener memoria separada, pueden evitar conflictos en la caché del CPU.

- **Ruido del sistema:** Otros programas en ejecución pueden influir más en los hilos que en los procesos.

En resumen, aunque los hilos son más eficientes en teoría, en ciertas condiciones reales los procesos pueden tener mejor rendimiento.

# ‼️Errores encontrados en la sincronización y su solución

Aunque inicialmente el programa no generaba los mismos números aleatorios en procesos e hilos, al introducir listas pre-generadas de detecciones aumentó la cantidad de veces que varios hilos o procesos intentaban actualizar el contador global simultáneamente. Esto provocó que se manifieste el clásico problema de *race condition*, en donde algunos incrementos se perdían al no existir un mecanismo de sincronización. La solución fue implementar correctamente mutex (para hilos) y semáforos (para procesos), garantizando exclusión mutua al modificar la variable compartida.

Otro problema que encontramos se debió a la semilla aleatoria global que se requería generar en un principio. Todos los hilos empezaron a competir para ver a quién se le asignaba primero el número aleatorio, generando *race condition* y ralentizando la ejecución de los hilos significativamente.

# ☯️ Diferencias encontradas

| Diferencia clave              | Procesos                            | Hilos                                                    |
| ----------------------------- | ----------------------------------- | -------------------------------------------------------- |
| *Creación*                    | fork() crea un proceso separado     | pthread_create() crea un hilo dentro del mismo proceso   |
| *Memoria compartida*          | Se requiere shmget, shmat, etc.     | Compartida automáticamente entre hilos                   |
| *Sincronización*              | Se usan *semáforos* (semop)         | Se usa *mutex* (pthread_mutex)                           |
| *Coste de contexto*           | Alto (más lento)                    | Bajo (más rápido)                                        |
| *Comunicación entre tareas*   | Compleja (IPC necesario)            | Simple (comparten variables)                             |

# 🔍 Análisis de cuándo usar hilos y cuándo usar procesos

| Aspecto                       | Hilos                                                 | Procesos                             |
| ----------------------------- | ----------------------------------------------------- | ------------------------------------ |
| *Compartir datos*             | Fácil (comparten memoria)                             | Difícil (requiere IPC)               |
| Aislamiento                   | Bajo (un hilo puede afectar a todo el proceso)        | Alto (fallos aislados)               |
| Creación y destrucción        | Rápida y barata                                       | Más lenta y costosa                  |
| Seguridad                     | Menor, hay que sincronizar mucho                      | Mayor, cada proceso es independiente |
| Complejidad de sincronización | Alta, cuidado con race conditions                     | Menor, pero comunicación más difícil |
| Escenarios ideales            | Tareas que requieren comunicación y datos compartidos | Tareas independientes o aisladas     |

---

# 🦉 Reflexión y conclusión final

El proyecto permitió comprender de manera práctica las diferencias fundamentales entre el uso de hilos y procesos en programación concurrente. Se evidenció que, aunque los hilos ofrecen ventajas en velocidad y eficiencia para compartir datos, requieren un manejo cuidadoso para evitar problemas como las condiciones de carrera. Por su parte, los procesos, al estar aislados, brindan mayor seguridad en la ejecución pero con un costo en rendimiento y comunicación. Estas observaciones resaltan que la elección entre hilos y procesos debe basarse en los requerimientos específicos del problema a resolver.

En general, la experiencia mostró que la programación concurrente no solo implica optimizar tiempos, sino también garantizar la integridad y estabilidad del programa. La práctica permitió desarrollar una base sólida para diseñar aplicaciones que empleen concurrencia de manera efectiva, tomando decisiones informadas sobre cuándo y cómo usar hilos o procesos. Este aprendizaje es fundamental para enfrentar proyectos futuros tanto académicos como profesionales, donde el manejo adecuado de recursos y la sincronización son clave para el éxito.
