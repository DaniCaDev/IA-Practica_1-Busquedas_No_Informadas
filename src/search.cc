#include "search.h"

#include <algorithm>
#include <deque>
#include <stack>

namespace {

// Reconstruye camino origin->goal con parent[v] = padre de v (o -1).
std::vector<int> ReconstructPath(int origin, int goal, const std::vector<int>& parent) {
  std::vector<int> path;
  if (goal < 0) return path;
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

}  // namespace

SearchResult UninformedSearch::Run(const Graph& g, int origin, int dest, Strategy strategy) {
  if (origin < 1 || dest < 1 || origin > static_cast<int>(g.NumVertices()) ||
      dest > static_cast<int>(g.NumVertices())) {
    return {};
  }
  return (strategy == Strategy::kBfs) ? Bfs(g, origin, dest) : Dfs(g, origin, dest);
}

double UninformedSearch::ComputePathCost(const Graph& g, const std::vector<int>& path) {
  if (path.size() < 2) return 0.0;
  double cost = 0.0;
  for (std::size_t i = 1; i < path.size(); ++i) {
    double w = g.EdgeCost(path[i - 1], path[i]);
    if (w < 0.0) return -1.0;
    cost += w;
  }
  return cost;
}

// ======================= BFS =======================
SearchResult UninformedSearch::Bfs(const Graph& g, int origin, int dest) {
  SearchResult res;

  const int n = static_cast<int>(g.NumVertices());
  std::vector<int> parent(n + 1, -1);
  std::vector<bool> visited(n + 1, false);

  std::deque<int> q;
  q.push_back(origin);
  visited[origin] = true;

  // Acumulados (con duplicados).
  std::vector<int> gen_acc;   // historial de “generados”.
  std::vector<int> insp_acc;  // historial de inspeccionados.

  // El origen cuenta como primer “generado”.
  gen_acc.push_back(origin);

  while (!q.empty()) {
    int u = q.front();
    q.pop_front();

    // --- Snapshot al inicio de la iteración (antes de expandir u) ---
    res.expanded_nodes.push_back(u);
    res.generated_acc.push_back(gen_acc);   // acumulado hasta ahora (con duplicados)
    res.inspected_acc.push_back(insp_acc);  // inspeccionados hasta ahora

    // Inspeccionamos u
    insp_acc.push_back(u);

    // Parada inmediata si el inspeccionado es el destino (criterio profesora)
    if (u == dest) {
      // Iteración sin sucesores (no se expanden más)
      res.successors_step.push_back({});
      res.enqueued_step.push_back({});

      // Snapshot final “post-inspección” (estilo de la foto negra)
      res.expanded_nodes.push_back(u);
      res.generated_acc.push_back(gen_acc);   // generados no cambian
      res.inspected_acc.push_back(insp_acc);  // ahora incluye al destino

      res.path = ReconstructPath(origin, dest, parent);
      res.total_cost = ComputePathCost(g, res.path);
      res.found = true;
      return res;
    }

    // Generamos sucesores de u
    std::vector<int> succ_this;
    std::vector<int> enq_this;

    auto neigh = g.Neighbors(u);
    std::sort(neigh.begin(), neigh.end());  // orden determinista

    int parent_u = parent[u];  // -1 en la raíz
    for (int v : neigh) {
      succ_this.push_back(v);            // se muestran todos los sucesores
      if (v != parent_u) gen_acc.push_back(v);  // acumulado con duplicados (excluye padre)
      if (!visited[v]) {
        visited[v] = true;
        parent[v] = u;
        q.push_back(v);
        enq_this.push_back(v);
      }
    }

    res.successors_step.push_back(succ_this);
    res.enqueued_step.push_back(enq_this);
  }

  return res;  // no encontrado
}

// ======================= DFS =======================
SearchResult UninformedSearch::Dfs(const Graph& g, int origin, int dest) {
  SearchResult res;

  const int n = static_cast<int>(g.NumVertices());
  std::vector<int> parent(n + 1, -1);
  std::vector<bool> visited(n + 1, false);

  std::stack<int> st;
  st.push(origin);

  std::vector<int> gen_acc;
  std::vector<int> insp_acc;

  gen_acc.push_back(origin);

  while (!st.empty()) {
    int u = st.top();
    st.pop();

    if (visited[u]) {
      // Descartes por ya visitado no cuentan como iteración.
      continue;
    }

    // --- Snapshot al inicio de la iteración ---
    res.expanded_nodes.push_back(u);
    res.generated_acc.push_back(gen_acc);
    res.inspected_acc.push_back(insp_acc);

    // Inspeccionar u
    visited[u] = true;
    insp_acc.push_back(u);

    // Parada inmediata si u es el destino
    if (u == dest) {
      res.successors_step.push_back({});
      res.enqueued_step.push_back({});

      // Snapshot final “post-inspección”
      res.expanded_nodes.push_back(u);
      res.generated_acc.push_back(gen_acc);
      res.inspected_acc.push_back(insp_acc);

      res.path = ReconstructPath(origin, dest, parent);
      res.total_cost = ComputePathCost(g, res.path);
      res.found = true;
      return res;
    }

    // Sucesores; para DFS apilamos en orden inverso tras ordenar
    auto neigh = g.Neighbors(u);
    std::sort(neigh.begin(), neigh.end());
    std::vector<int> succ_this;
    std::vector<int> enq_this;

    int parent_u = parent[u];
    for (auto it = neigh.rbegin(); it != neigh.rend(); ++it) {
      int v = *it;
      succ_this.push_back(v);
      if (v != parent_u) gen_acc.push_back(v);  // acumulado con duplicados (excluye padre)
      if (!visited[v]) {
        if (parent[v] == -1) parent[v] = u;
        st.push(v);
        enq_this.push_back(v);
      }
    }

    res.successors_step.push_back(succ_this);
    res.enqueued_step.push_back(enq_this);
  }

  return res;
}

