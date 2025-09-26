#include "search.h"

#include <deque>
#include <stack>
#include <unordered_set>
#include <algorithm>

namespace {

// Devuelve el camino reconstruido desde 'goal' hacia atrás con el vector parent.
std::vector<int> ReconstructPath(int origin, int goal, const std::vector<int>& parent) {
  std::vector<int> path;
  if (goal <= 0) return path;
  int cur = goal;
  while (cur != -1) {
    path.push_back(cur);
    if (cur == origin) break;
    cur = parent[cur];
  }
  if (path.empty() || path.back() != origin) return {};
  std::reverse(path.begin(), path.end());
  return path;
}

// Convierte un contenedor a vector<int> para volcar en las trazas (frontera).
template <typename Iterable>
std::vector<int> ToVectorInt(const Iterable& c) {
  std::vector<int> out;
  for (const auto& x : c) out.push_back(x);
  return out;
}

}  // namespace

SearchResult UninformedSearch::Run(const Graph& g, int origin, int dest, Strategy strategy) {
  if (origin < 1 || dest < 1 || origin > static_cast<int>(g.NumVertices()) ||
      dest > static_cast<int>(g.NumVertices())) {
    return {};
  }
  if (strategy == Strategy::kBfs) return Bfs(g, origin, dest);
  return Dfs(g, origin, dest);
}

double UninformedSearch::ComputePathCost(const Graph& g, const std::vector<int>& path) {
  if (path.size() < 2) return 0.0;
  double cost = 0.0;
  for (std::size_t i = 1; i < path.size(); ++i) {
    double w = g.EdgeCost(path[i - 1], path[i]);
    if (w < 0.0) return -1.0;  // arista inexistente
    cost += w;
  }
  return cost;
}

SearchResult UninformedSearch::Bfs(const Graph& g, int origin, int dest) {
  SearchResult res;
  res.found = false;

  const int n = static_cast<int>(g.NumVertices());
  std::vector<int> parent(n + 1, -1);
  std::vector<bool> visited(n + 1, false);

  std::deque<int> q;
  q.push_back(origin);
  visited[origin] = true;

  std::vector<int> visited_list;  // para trazar "nodos inspeccionados"

  // Iteración 1: frontera inicial (solo el origen), y aún sin inspeccionados
  res.frontier_steps.push_back(ToVectorInt(q));
  res.visited_steps.push_back(visited_list);

  while (!q.empty()) {
    int u = q.front();
    q.pop_front();

    // Añadimos a inspeccionados y registramos la frontera tras sacar u
    visited_list.push_back(u);
    res.frontier_steps.push_back(ToVectorInt(q));
    res.visited_steps.push_back(visited_list);

    if (u == dest) {
      res.path = ReconstructPath(origin, dest, parent);
      res.total_cost = ComputePathCost(g, res.path);
      res.found = true;
      return res;
    }

    for (int v : g.Neighbors(u)) {
      if (!visited[v]) {
        visited[v] = true;
        parent[v] = u;
        q.push_back(v);
      }
    }
    // Tras generar vecinos de u, registramos el estado (frontera y visitados).
    res.frontier_steps.push_back(ToVectorInt(q));
    res.visited_steps.push_back(visited_list);
  }

  return res;  // no encontrado
}

SearchResult UninformedSearch::Dfs(const Graph& g, int origin, int dest) {
  SearchResult res;
  res.found = false;

  const int n = static_cast<int>(g.NumVertices());
  std::vector<int> parent(n + 1, -1);
  std::vector<bool> visited(n + 1, false);

  std::stack<int> st;
  st.push(origin);

  std::vector<int> visited_list;

  // Iteración 1: frontera inicial (origen en la pila), sin inspeccionados
  // Para mostrar la pila como "frontera" en orden LIFO, la volcamos a vector.
  {
    std::stack<int> tmp = st;
    std::vector<int> frontier;
    while (!tmp.empty()) {
      frontier.push_back(tmp.top());
      tmp.pop();
    }
    res.frontier_steps.push_back(frontier);
    res.visited_steps.push_back(visited_list);
  }

  while (!st.empty()) {
    int u = st.top();
    st.pop();

    if (visited[u]) {
      // Registrar estado tras descartar repetidos
      std::stack<int> tmp = st;
      std::vector<int> frontier;
      while (!tmp.empty()) {
        frontier.push_back(tmp.top());
        tmp.pop();
      }
      res.frontier_steps.push_back(frontier);
      res.visited_steps.push_back(visited_list);
      continue;
    }

    visited[u] = true;
    visited_list.push_back(u);

    // Registrar tras inspeccionar u
    {
      std::stack<int> tmp = st;
      std::vector<int> frontier;
      while (!tmp.empty()) {
        frontier.push_back(tmp.top());
        tmp.pop();
      }
      res.frontier_steps.push_back(frontier);
      res.visited_steps.push_back(visited_list);
    }

    if (u == dest) {
      res.path = ReconstructPath(origin, dest, parent);
      res.total_cost = ComputePathCost(g, res.path);
      res.found = true;
      return res;
    }

    // En DFS apilamos vecinos. Para un orden "natural", apilamos en reversa
    // para que el menor índice salga antes (opcional y determinista).
    const auto& neigh = g.Neighbors(u);
    for (auto it = neigh.rbegin(); it != neigh.rend(); ++it) {
      int v = *it;
      if (!visited[v]) {
        if (parent[v] == -1) parent[v] = u;
        st.push(v);
      }
    }

    // Registrar estado tras generar
    {
      std::stack<int> tmp = st;
      std::vector<int> frontier;
      while (!tmp.empty()) {
        frontier.push_back(tmp.top());
        tmp.pop();
      }
      res.frontier_steps.push_back(frontier);
      res.visited_steps.push_back(visited_list);
    }
  }

  return res;  // no encontrado
}
