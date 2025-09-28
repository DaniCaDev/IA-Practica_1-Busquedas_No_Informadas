#ifndef IA_PRACTICE_NODE_H_
#define IA_PRACTICE_NODE_H_

#include <cstddef>

/**
 * @brief Nodo de búsqueda (no del grafo), usado por BFS/DFS.
 *
 * - id: identificador de vértice (1..n).
 * - parent: id del padre en el árbol de búsqueda, o -1 si es raíz.
 * - depth: profundidad desde el origen (0 en la raíz).
 * - g_cost: coste acumulado desde el origen (opcional para informe).
 */
class Node {
 public:
  Node() = default;
  Node(int id, int parent, int depth, double g_cost)
      : id_(id), parent_(parent), depth_(depth), g_cost_(g_cost) {}

  int id() const { return id_; }
  int parent() const { return parent_; }
  int depth() const { return depth_; }
  double g_cost() const { return g_cost_; }

  void set_parent(int p) { parent_ = p; }
  void set_depth(int d) { depth_ = d; }
  void set_g_cost(double g) { g_cost_ = g; }

 private:
  int id_ = -1;
  int parent_ = -1;
  int depth_ = 0;
  double g_cost_ = 0.0;
};

#endif  // IA_PRACTICE_NODE_H_
