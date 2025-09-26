#ifndef IA_PRACTICE_GRAPH_H_
#define IA_PRACTICE_GRAPH_H_

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Representa un grafo no dirigido con matriz de costes y lista de adyacencia.
 *
 * El formato del fichero de entrada sigue exactamente el del guion:
 * - Primera línea: n (número de vértices)
 * - A continuación n(n-1)/2 distancias d(i,j) en orden lexicográfico por pares (i<j):
 *   (1,2), (1,3), ..., (1,n), (2,3), ..., (n-1,n)
 * - d(i,i) = 0 (implícito), d(i,j) = d(j,i), y d(i,j) = -1 si no hay arista.
 */
class Graph {
 public:
  Graph() = default;

  /**
   * @brief Carga el grafo desde un fichero de texto con el formato del guion.
   * @param path Ruta del fichero.
   * @return true si se carga con éxito, false en caso contrario.
   */
  bool LoadFromFile(const std::string& path);

  /**
   * @brief Número de vértices (1..n).
   */
  std::size_t NumVertices() const { return n_; }

  /**
   * @brief Devuelve los vecinos (ids 1..n) accesibles desde v (1-based).
   */
  const std::vector<int>& Neighbors(int v) const;

  /**
   * @brief Coste de la arista (u,v). Devuelve -1.0 si no existe.
   */
  double EdgeCost(int u, int v) const;

  /**
   * @brief Número de aristas (no dirigidas) presentes (con coste >= 0).
   */
  std::size_t NumEdges() const { return m_; }

 private:
  void BuildAdjacencyList();

  std::size_t n_ = 0;  // número de vértices
  std::size_t m_ = 0;  // número de aristas no dirigidas
  std::vector<std::vector<double>> adj_matrix_;  // [n][n], -1 si no hay arista
  std::vector<std::vector<int>> adj_list_;       // vecinos por índice 1..n
};

#endif  // IA_PRACTICE_GRAPH_H_
