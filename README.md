## SUKP - Set-Union Knapsack Problem

Este proyecto es una implementación del **Set-Union Knapsack Problem (SUKP)**, una variante del clásico **problema de la mochila**.
La idea principal es maximizar la ganancia total sin pasar la capacidad máxima, pero considerando una **asignación de ganancia para cada elemento**.

## Descripción

En este caso, el problema se aborda **desde la perspectiva de los elementos**, es decir:
- Se distribuye la **ganancia** entre los distintos elementos.
- Luego se calcula una **eficacia** dividiendo la ganancia por el peso.
- Finalmente, según esta eficacia, se eligen los elementos más convenientes para llenar la mocihla.

El código incluye **tres algoritmos diferentes** para resolver el problema:

1. **Greedy (determinista):**
   Selecciona los elementos más eficientes hasta que no haya más espacio.
   Si bien es rápido, pero no siempre entrega la mejor solución.

2. **Probabilístico:**
   Toma decisiones con cierta aleatoridad, considerando la eficacia de cada elemento.
   Esto permite explorar soluciones diferentes y salir de los óptimos locales.

3. **LNS (Large Neighborhood Search):**
   Parte de una solución inicial (por ejemplo, la del método greedy) y busca mejoras explorando "vecindarios" mas grandes, reemplazando o intercambiando varios elementos.

## Resultados obtenidos

En el archivo ['resultados_sukp.pdf'](./resultados_sukp.pdf) se incluyen las tablas con las **ganancias totales obtenidas por cada método**, asi como comparaciones entre ellos.
En el se puede observar las diferencias de rendimiento entre los distintos enfoques.

## Compilación
Para compilar el programa, usa el siguiente comando:

gcc sukp.c -o sukp -lm
