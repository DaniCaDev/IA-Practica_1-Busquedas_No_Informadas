#include "search.h"

#include <algorithm>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <random> 


namespace {

// Reconstruye camino origin->goal con parent[v] = padre de v (o -1).
std::vector<int> ReconstructPath(int origin, int goal, const std::vector<int>& parent) {
  std::vector<int> path;
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

//SearchResult UninformedSearch::Run(const Graph& g, int origin, int dest, Strategy strategy) {
//  if (origin < 1 || dest < 1 || origin > static_cast<int>(g.NumVertices()) ||
//      dest > static_cast<int>(g.NumVertices())) {
//    return {};
//  }
//  return (strategy == Strategy::kBfs) ? Bfs(g, origin, dest) : Dfs(g, origin, dest);
//}


SearchResult UninformedSearch::Run(const Graph& g, int origin, int dest, Strategy strategy) {
  if (origin < 1 || dest < 1 ||
      origin > static_cast<int>(g.NumVertices()) ||
      dest   > static_cast<int>(g.NumVertices())) {
    return {};
  }
  switch (strategy) {
    case Strategy::kBfs:      return Bfs(g, origin, dest);
    case Strategy::kDfs:      return Dfs(g, origin, dest);
    case Strategy::kBfsMulti: return BfsMulti(g, origin, dest, 10); // Ponemos el máximo run 
  }
   return {};
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
  std::vector<bool> discovered(n + 1, false);  // marcado al ENCOLAR
  std::vector<bool> inspected(n + 1, false);   // inspeccionados (una sola vez)

  std::deque<int> q;

  std::vector<int> gen_acc;   // acumulado con duplicados (excluye padre)
  std::vector<int> insp_acc;  // inspeccionados acumulado

  // Iteración 1: encolamos origen y lo marcamos descubierto
  q.push_back(origin);
  discovered[origin] = true;
  parent[origin] = -1;
  gen_acc.push_back(origin);

  while (!q.empty()) {
    int u = q.front();
    q.pop_front();

    if (inspected[u]) continue;       // nunca inspeccionamos dos veces el mismo nodo
    inspected[u] = true;

    // Snapshot AL INICIO de la iteración
    res.expanded_nodes.push_back(u);
    res.generated_acc.push_back(gen_acc);
    res.inspected_acc.push_back(insp_acc);

    // Inspeccionamos u
    insp_acc.push_back(u);

    if (u == dest) {
      res.path = ReconstructPath(origin, dest, parent);
      res.total_cost = ComputePathCost(g, res.path);
      res.found = true;
      return res;
    }

    // Generar sucesores en orden ascendente
    auto neigh = g.Neighbors(u);
    std::sort(neigh.begin(), neigh.end());

    std::vector<int> succ_this, enq_this;
    for (int v : neigh) {
      succ_this.push_back(v);
      if (v != parent[u]) gen_acc.push_back(v);  // “generados” con duplicados (excluye padre)

      if (!discovered[v]) {
        discovered[v] = true;   // marcar al ENCOLAR evita duplicados en cola
        parent[v] = u;
        q.push_back(v);
        enq_this.push_back(v);
      }
    }

    res.successors_step.push_back(succ_this);
    res.enqueued_step.push_back(enq_this);
  }

  return res;
}

// ======================= DFS =======================

// Estado = (nodo, padre) para permitir reinspecciones por ramas distintas.
struct State { int node; int parent; };
struct StateHash {
  std::size_t operator()(const State& s) const noexcept {
    return (static_cast<std::size_t>(s.node) << 20) ^ static_cast<std::size_t>(s.parent + 1);
  }
};
struct StateEq {
  bool operator()(const State& a, const State& b) const noexcept {
    return a.node == b.node && a.parent == b.parent;
  }
};

static std::vector<int> ReconstructPathFromStates(
    int origin, State cur,
    const std::unordered_map<State, State, StateHash, StateEq>& pred) {
  std::vector<int> rev; rev.push_back(cur.node);
  State s = cur;
  while (!(s.node == origin && s.parent == -1)) {
    auto it = pred.find(s);
    if (it == pred.end()) break;
    State p = it->second;
    rev.push_back(p.node);
    s = p;
  }
  std::reverse(rev.begin(), rev.end());
  return rev;
}

SearchResult UninformedSearch::Dfs(const Graph& g, int origin, int dest) {
  SearchResult res;

  // -------- Estado en la pila: (nodo actual, padre desde el que llego) --------
  struct S { int u; int p; };

  const int n = static_cast<int>(g.NumVertices());
  std::vector<int> parent(n + 1, -1);
  std::vector<bool> inspected(n + 1, false);   // asegura: cada nodo se inspecciona UNA sola vez

  std::stack<S> st;

  // Acumulados que imprime tu DumpReport
  std::vector<int> gen_acc;   // “Generados” acumulado con duplicados (excluye padre)
  std::vector<int> insp_acc;  // “Inspeccionados” acumulado (sin repetidos)

  // Iteración 1: metemos la raíz; NO marcamos descubierto aquí (permitimos duplicados en pila)
  st.push({origin, -1});
  gen_acc.push_back(origin);

  while (!st.empty()) {
    S cur = st.top();
    st.pop();

    // Si este nodo ya fue inspeccionado, lo ignoramos (no cuenta como iteración)
    if (inspected[cur.u]) continue;

    // ----------  AL INICIO de la iteración (antes de generar sucesores) ----------
    res.expanded_nodes.push_back(cur.u);
    res.generated_acc.push_back(gen_acc);
    res.inspected_acc.push_back(insp_acc);

    // Inspeccionamos por PRIMERA vez este nodo
    inspected[cur.u] = true;
    if (parent[cur.u] == -1) parent[cur.u] = cur.p;  // fijamos el padre desde el estado
    insp_acc.push_back(cur.u);

    // ¿Destino?
    if (cur.u == dest) {
      res.path = ReconstructPath(origin, dest, parent);
      res.total_cost = ComputePathCost(g, res.path);
      res.found = true;
      return res;  // sin snapshot extra (tal como te piden)
    }

    // === CLAVE: no ordenamos vecinos; usamos el orden natural que da Graph::Neighbors(u) ===
    const auto& neigh = g.Neighbors(cur.u);

    // 1) Actualizar “Generados” (acumulado con duplicados), excluyendo al padre directo
    for (int v : neigh) {
      if (v != parent[cur.u]) gen_acc.push_back(v);
    }

    // 2) APILAR en orden inverso al natural para recorrerlos luego en su orden natural (LIFO)
    for (auto it = neigh.rbegin(); it != neigh.rend(); ++it) {
      int v = *it;
      if (v == parent[cur.u]) continue;  // evita vuelta inmediata al padre
      st.push({v, cur.u});               // permitimos duplicados en pila; se filtrarán en 'inspected'
    }
  }

  return res;  // no encontrado
}


SearchResult UninformedSearch::BfsMulti(const Graph& g, int origin, int dest, int max_runs) {
  SearchResult best;  // devolveremos el primer éxito; si no, queda vacío

  if (origin == dest) {
    best.path = {origin};
    best.total_cost = 0.0;
    best.found = true;
    return best;
  }
  const auto& root_neigh = g.Neighbors(origin);
  if (root_neigh.empty()) return best;  // sin hijos del origen, imposible

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> pick(0, static_cast<int>(root_neigh.size()) - 1);

  for (int run = 0; run < max_runs; ++run) {
    int child = root_neigh[pick(gen)];

    // Estructuras del BFS estándar
    const int n = static_cast<int>(g.NumVertices());
    std::vector<int> parent(n + 1, -1);
    std::vector<bool> discovered(n + 1, false);  // marcar al ENCOLAR
    std::vector<bool> inspected(n + 1, false);   // no inspeccionar dos veces

    std::deque<int> q;

    // Acumulados para el informe (se resetean en cada ejecución)
    std::vector<int> gen_acc;
    std::vector<int> insp_acc;

    // -------- Iteración de "inspección del origen" (no se expanden sus sucesores) --------
    // Iteración 1 del run: snapshot del origen
    best.expanded_nodes.push_back(origin);
    gen_acc.push_back(origin);
    best.generated_acc.push_back(gen_acc);
    best.inspected_acc.push_back(insp_acc);
    // Marcar inspeccionado el origen en este run
    inspected[origin] = true;
    parent[origin] = -1;
    insp_acc.push_back(origin);

    // -------- Arranque BFS desde un hijo aleatorio del origen --------
    // Lo contamos como "generado" y lo encolamos para procesarlo
    gen_acc.push_back(child);
    discovered[child] = true;
    parent[child] = origin;
    q.push_back(child);
    while (!q.empty()) {
      int u = q.front(); q.pop_front();
      if (inspected[u]) continue;
      inspected[u] = true;
      // Snapshot AL INICIO de la iteración (antes de generar sucesores)
      best.expanded_nodes.push_back(u);
      best.generated_acc.push_back(gen_acc);
      best.inspected_acc.push_back(insp_acc);
      // Inspeccionamos u
      insp_acc.push_back(u);
      if (u == dest) {
        // Camino y coste
        best.path = ReconstructPath(origin, dest, parent);
        best.total_cost = ComputePathCost(g, best.path);
        best.found = true;
        return best;  // éxito: devolvemos inmediatamente
      }
      // Generar sucesores en orden natural; “generados” con duplicados (excluye el padre)
      const auto& neigh = g.Neighbors(u);

      std::vector<int> succ_this, enq_this;
      for (int v : neigh) {
        succ_this.push_back(v);
        if (v != parent[u]) gen_acc.push_back(v);
        if (!discovered[v]) {
          discovered[v] = true;
          parent[v] = u;
          q.push_back(v);
          enq_this.push_back(v);
        }
      }

      best.successors_step.push_back(succ_this);
      best.enqueued_step.push_back(enq_this);
    }
    // Si no se encontró en este run, se repite el bucle escogiendo otro hijo al azar.
    // (Las trazas de los runs se concatenan en 'best' tal y como están; esto no afecta al camino/coste).
  }
  // No encontrado tras max_runs
  return best;
}


