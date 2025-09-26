#ifndef IA_PRACTICE_SEARCH_H_
#define IA_PRACTICE_SEARCH_H_

#include <string>
#include <vector>

#include "graph.h"

/**
 * @brief Estrategia de búsqueda no informada.
 */
enum class Strategy { kBfs, kDfs };

/**
 * @brief Resultado completo de la búsqueda, con trazas por iteración.
 */
struct SearchResult {
  std::vector<int> path;                           ///< Camino desde origen a destino (1..n).
  double total_cost = -1.0;                        ///< Suma de costes del camino o -1 si no encontrado.
  std::vector<std::vector<int>> frontier_steps;    ///< Frontera (nodos generados) por iteración.
  std::vector<std::vector<int>> visited_steps;     ///< Visitados (nodos inspeccionados) por iteración.
  bool found = false;                              ///< true si se halló camino.
};

/**
 * @brief Implementa búsquedas en amplitud y profundidad sobre un grafo no dirigido.
 */
class UninformedSearch {
 public:
  /**
   * @brief Ejecuta BFS o DFS para hallar un camino entre origen y destino.
   * @param g Grafo.
   * @param origin Vértice origen (1..n).
   * @param dest Vértice destino (1..n).
   * @param strategy Estrategia: BFS o DFS.
   * @return SearchResult con camino, coste y trazas por iteración.
   */
  static SearchResult Run(const Graph& g, int origin, int dest, Strategy strategy);

  /**
   * @brief Reconstruye el coste total de un camino en el grafo.
   * @param g Grafo.
   * @param path Camino (1..n).
   * @return Suma de costes o -1.0 si el camino no es válido.
   */
  static double ComputePathCost(const Graph& g, const std::vector<int>& path);

 private:
  static SearchResult Bfs(const Graph& g, int origin, int dest);
  static SearchResult Dfs(const Graph& g, int origin, int dest);
};

#endif  // IA_PRACTICE_SEARCH_H_
