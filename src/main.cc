#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "graph.h"
#include "search.h"

namespace {

struct CliOptions {
  std::string input_path;
  std::optional<int> origin;
  std::optional<int> dest;
  std::string strategy = "bfs";  // "bfs" o "dfs"
  std::string output_path = "resultado.txt";
};

void PrintMenu() {
  std::cout << "=== Busquedas no informadas (BFS/DFS) ===\n";
  std::cout << "1) Cargar grafo desde fichero\n";
  std::cout << "2) Elegir origen y destino\n";
  std::cout << "3) Elegir estrategia (bfs/dfs)\n";
  std::cout << "4) Ejecutar y volcar resultados a fichero\n";
  std::cout << "5) Salir\n";
}

bool ParseArgs(int argc, char* argv[], CliOptions* opts) {
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--input" && i + 1 < argc) {
      opts->input_path = argv[++i];
    } else if (a == "--origin" && i + 1 < argc) {
      opts->origin = std::stoi(argv[++i]);
    } else if (a == "--dest" && i + 1 < argc) {
      opts->dest = std::stoi(argv[++i]);
    } else if (a == "--strategy" && i + 1 < argc) {
      opts->strategy = argv[++i];
    } else if (a == "--output" && i + 1 < argc) {
      opts->output_path = argv[++i];
    } else {
      std::cerr << "Argumento desconocido o incompleto: " << a << "\n";
      return false;
    }
  }
  return true;
}

//Strategy StrategyFromString(const std::string& s) {
//  if (s == "dfs" || s == "DFS") return Strategy::kDfs;
//  return Strategy::kBfs;
//}

Strategy StrategyFromString(const std::string& s) {
  if (s == "dfs" || s == "DFS") return Strategy::kDfs;
  if (s == "bfs-multi" || s == "BFS-MULTI" || s == "bfsmulti" || s == "BFSMULTI")
    return Strategy::kBfsMulti;
  return Strategy::kBfs;
}


// Informe bonito (estilo propio: corchetes y separadores).
void DumpReport(const Graph& g, int origin, int dest, const SearchResult& r,
                const std::string& out_path, Strategy st) {
  std::ofstream out(out_path);
  if (!out) {
    std::cerr << "Error: no se pudo abrir el fichero de salida: " << out_path << "\n";
    return;
  }

  out << "============================================\n";
  out << "         EJECUCION " << (st == Strategy::kBfs ? "BFS" : "DFS") << "\n";
  out << "============================================\n\n";

  out << "Vertices del grafo : " << g.NumVertices() << "\n";
  out << "Aristas del grafo  : " << g.NumEdges() << "\n";
  out << "Origen             : " << origin << "\n";
  out << "Destino            : " << dest << "\n";
  out << "--------------------------------------------\n\n";

  const std::size_t steps = r.expanded_nodes.size();
  for (std::size_t i = 0; i < steps; ++i) {
    out << "----------- Iteracion " << (i + 1) << " -----------\n";

    // Generados acumulados (con duplicados)
    out << "Nodos generados: [ ";
    if (i < r.generated_acc.size() && !r.generated_acc[i].empty()) {
      for (std::size_t j = 0; j < r.generated_acc[i].size(); ++j) {
        if (j) out << ", ";
        out << r.generated_acc[i][j];
      }
    } else {
      out << "-";
    }
    out << " ]\n";

    // Inspeccionados acumulados
    out << "Nodos inspeccionados: [ ";
    if (i < r.inspected_acc.size() && !r.inspected_acc[i].empty()) {
      for (std::size_t j = 0; j < r.inspected_acc[i].size(); ++j) {
        if (j) out << ", ";
        out << r.inspected_acc[i][j];
      }
    } else {
      out << "-";
    }
    out << " ]\n";

    // Nodo expandido y sucesores
    out << ">> Nodo expandido: " << r.expanded_nodes[i] << "\n";
    out << "   Sucesores: [ ";
    if (i < r.successors_step.size()) {
      for (std::size_t j = 0; j < r.successors_step[i].size(); ++j) {
        if (j) out << ", ";
        out << r.successors_step[i][j];
      }
    }
    out << " ]\n";

    // Comentarios por sucesor (si no se encola o si es destino)
    auto was_enqueued = [&](int v) -> bool {
      if (i >= r.enqueued_step.size()) return false;
      return std::find(r.enqueued_step[i].begin(), r.enqueued_step[i].end(), v) !=
             r.enqueued_step[i].end();
    };
    if (i < r.successors_step.size()) {
      for (int v : r.successors_step[i]) {
        if (v == dest) {
          out << "   -> El nodo " << v << " es el OBJETIVO alcanzado!\n";
        } else if (!was_enqueued(v)) {
          out << "   -> El nodo " << v << " no se añade (ya visitado o es padre).\n";
        }
      }
    }

    out << "============================================\n\n";
  }

  if (r.found) {
    out << "============= CAMINO ENCONTRADO =============\n";
    for (std::size_t i = 0; i < r.path.size(); ++i) {
      if (i) out << " -> ";
      out << r.path[i];
    }
    out << "\nCoste total: " << std::fixed << std::setprecision(2) << r.total_cost << "\n";
  } else {
    out << "============= CAMINO NO ENCONTRADO =============\n";
  }

  out << "============================================\n";
  std::cout << "Informe generado en: " << out_path << "\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  CliOptions opts;
  ParseArgs(argc, argv, &opts);

  Graph g;

  // Modo por parámetros
  if (!opts.input_path.empty() && g.LoadFromFile(opts.input_path)) {
    if (opts.origin.has_value() && opts.dest.has_value()) {
      Strategy st = StrategyFromString(opts.strategy);
      SearchResult res = UninformedSearch::Run(g, *opts.origin, *opts.dest, st);
      if (res.found) res.total_cost = UninformedSearch::ComputePathCost(g, res.path);
      DumpReport(g, *opts.origin, *opts.dest, res, opts.output_path, st);
      return EXIT_SUCCESS;
    }
  }

  // Modo interactivo
  std::string path;
  int origin = -1, dest = -1;
  std::string strat = "dfs";        // por defecto DFS (puedes cambiarlo)
  std::string out_path = "resultado.txt";

  while (true) {
    PrintMenu();
    std::cout << "Opcion: ";
    int op; if (!(std::cin >> op)) break;

    if (op == 1) {
      std::cout << "Ruta del fichero: ";
      std::cin >> path;
      if (!g.LoadFromFile(path)) {
        std::cout << "Error cargando grafo.\n";
      } else {
        std::cout << "Grafo cargado. n=" << g.NumVertices()
                << " m=" << g.NumEdges() << "\n";
      }
    } else if (op == 2) {
      std::cout << "Origen: "; std::cin >> origin;
      std::cout << "Destino: "; std::cin >> dest;
    } else if (op == 3) {
      std::cout << "Estrategia [dfs | bfs | bfs-multi]: ";
      std::cin >> strat;
      Strategy st = StrategyFromString(strat);
      std::cout << "Estrategia seleccionada: "
              << (st == Strategy::kDfs ? "DFS" : (st == Strategy::kBfs ? "BFS" : "BFS-MULTI"))
              << "\n";
    } else if (op == 4) {
      if (path.empty() || origin < 1 || dest < 1) {
        std::cout << "Faltan datos (grafo/origen/destino).\n";
        continue;
      }
      Strategy st = StrategyFromString(strat);
      std::cout << "Ejecutando " << (st == Strategy::kDfs ? "DFS" : (st == Strategy::kBfs ? "BFS" : "BFS-MULTI")) << "...\n";
      SearchResult r = UninformedSearch::Run(g, origin, dest, st);
      if (r.found) r.total_cost = UninformedSearch::ComputePathCost(g, r.path);
      DumpReport(g, origin, dest, r, out_path, st);
      std::cout << "Informe generado en: " << out_path << "\n";
    } else if (op == 5) {
      break;
    } else {
      std::cout << "Opcion invalida.\n";
    }
  }
}