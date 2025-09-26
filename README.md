# IA-Práctica_1-Búsquedas_No_Informadas
Práctica 1 de la asignatura Inteligencía Artificial en la que tenemos un problema acerca de las búsquedas no informadas
📘 Práctica: Búsquedas no informadas (BFS y DFS)

Este proyecto implementa algoritmos de búsqueda no informada (en anchura BFS y en profundidad DFS) sobre grafos no dirigidos con pesos, siguiendo el formato de entrada especificado en el guion de la práctica.

El programa permite:

Cargar un grafo desde un fichero de texto.

Seleccionar un vértice de origen y uno de destino.

Escoger la estrategia de búsqueda (BFS o DFS).

Generar un informe (resultado.txt) con:

Número de nodos y aristas.

Origen y destino.

Iteraciones (frontera y nodos inspeccionados en cada paso).

Camino encontrado y su coste total.

📂 Estructura de directorios

tu-proyecto/
├── Makefile
├── README.md
├── include/
│ ├── graph.h
│ └── search.h
├── src/
│ ├── graph.cc
│ ├── search.cc
│ └── main.cc
├── data/
│ ├── Grafo1.txt # Ejemplo de grafo de la práctica
│ └── Grafo2.txt # (otros grafos opcionales)
├── build/ # (se crea al compilar, contiene objetos .o y dependencias)
├── bin/ # (se crea al compilar, contiene el ejecutable final)
└── resultado.txt # (se genera tras ejecutar el programa)

⚙️ Compilación

El proyecto incluye un Makefile para compilar fácilmente:

make → Compilación en modo release (por defecto)
make debug → Compilación en modo debug (con símbolos de depuración)
make clean → Limpiar objetos y dependencias
make distclean → Limpiar todo (objetos y binario)

El ejecutable se genera en:
bin/busquedas

▶️ Ejecución

El programa puede ejecutarse de dos formas:

1) Modo interactivo (con menú)

./bin/busquedas

El menú permite:

Cargar grafo desde fichero (data/Grafo1.txt).

Definir vértice de origen y destino.

Seleccionar estrategia de búsqueda (bfs / dfs).

Ejecutar y generar el informe (resultado.txt).

Salir.

Nota: Cuando pida ruta del fichero, escribe por ejemplo:
data/Grafo1.txt

2) Modo por parámetros

./bin/busquedas --input data/Grafo1.txt --origin 1 --dest 4 --strategy bfs --output resultado.txt

--input: ruta del fichero de grafo.

--origin: vértice de origen (1..n).

--dest: vértice de destino (1..n).

--strategy: estrategia (bfs o dfs).

--output: fichero de salida (informe).

📑 Formato de entrada (grafo)

El fichero del grafo debe seguir exactamente el formato del guion:

Primera línea: n (número de vértices).

A continuación: n(n-1)/2 distancias d(i,j) en el orden:
(1,2), (1,3), ..., (1,n), (2,3), ..., (n-1,n)

Se asume:

d(i,i) = 0 (implícito).

d(i,j) = d(j,i) (simétrico).

-1 significa que no hay arista.

Ejemplo (extraído del guion):
4
9 7 5 3 6 1

📜 Ejemplo de ejecución

Entrada: data/Grafo1.txt con 15 nodos.
Ejecución BFS con origen 1 y destino 4:

./bin/busquedas --input data/Grafo1.txt --origin 1 --dest 4 --strategy bfs --output resultado.txt

Salida (resultado.txt):

Numero de nodos del grafo: 15
Numero de aristas del grafo: 19

Vertice origen: 1
Vertice destino: 4

Iteracion 1
Nodos generados: 1
Nodos inspeccionados: -

Iteracion 2
Nodos generados: 2, 3
Nodos inspeccionados: 1

...

Camino: 1 - 2 - 4
Costo: 15.00

🧑‍💻 Autor

Estudiante de 3º de Ingeniería Informática
Universidad de La Laguna
Implementado en C++17, con guía de estilo de Google C++