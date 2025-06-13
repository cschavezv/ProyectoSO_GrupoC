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
