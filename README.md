# IA-Tabu-Search-Greedy
## Solucion del Green-VRP usando Tabu Search y un algoritmo greedy para la solución inicial

### Instrucciones de compilación
Para compilar el código se debe ejecutar el siguiente comando en una terminal de linux:
``make``

### Instrucciones de ejecucion
Para ejecutar el programa se debe ejecutar el siguiente comando en una terminal linux:

``time ./G_VRP-Greedy+TS instances/<nombre_archivo> <numero_iteraciones>``

Donde <nombre_archivo> es la instancia del problema que se quiere resolver y <numero_iteraciones> corresponde al numero de veces que será ejecutado el algoritmo Tabu Search.

Ejemplo de uso: ``time ./G_VRP-Greedy+TS instances/Large\ VA\ Input_250c_21s.txt 1000``

### Consideraciones:
El tiempo total de ejecución del programa corresponde a la última sección del output, y se calcula sumando el tiempo asociado a real más el de user.

## Importante!!!
No ejecutar el programa con el archivo que lleva el nombre: 20c3sU2.txt, ya que por un error aleatorio el programa no puede ser ejecutado con esos datos.
