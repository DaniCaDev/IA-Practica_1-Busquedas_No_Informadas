#ifndef IA_PRACTICE_SEARCH_H_
#define IA_PRACTICE_SEARCH_H_

#include <string>
#include <vector>

#include "graph.h"

// Estrategia de búsqueda.
enum class Strategy { kBfs, kDfs, kBfsMulti };

// Resultado completo con trazas por iteración.
struct SearchResult {
  std::vector<int> path;                    // Camino final (1..n).
  double total_cost = -1.0;                 // Coste total del camino, -1 si no existe.

  // Una iteración = una expansión de un nodo.
  std::vector<int> expanded_nodes;                  // Nodo expandido en cada iteración.
  std::vector<std::vector<int>> successors_step;    // Sucesores vistos esa iteración.
  std::vector<std::vector<int>> enqueued_step;      // Sucesores realmente encolados/apilados.

  // Acumulados (con duplicados) que se imprimen en el informe.
  std::vector<std::vector<int>> generated_acc;      // Historial de “nodos generados” (con duplicados).
  std::vector<std::vector<int>> inspected_acc;      // Historial de “nodos inspeccionados”.

  bool found = false;                                // true si se halló un camino.
};

class UninformedSearch {
 public:
  static SearchResult Run(const Graph& g, int origin, int dest, Strategy strategy);
  static double ComputePathCost(const Graph& g, const std::vector<int>& path);

 private:
  static SearchResult Bfs(const Graph& g, int origin, int dest);
  static SearchResult Dfs(const Graph& g, int origin, int dest);
  static SearchResult BfsMulti(const Graph& g, int origin, int dest, int max_runs);
};

#endif  // IA_PRACTICE_SEARCH_H_

