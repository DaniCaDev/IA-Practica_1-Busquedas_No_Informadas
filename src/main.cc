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

Strategy StrategyFromString(const std::string& s) {
  if (s == "dfs" || s == "DFS") return Strategy::kDfs;
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
  std::string input_path;
  int origin = -1, dest = -1;
  std::string strategy = "bfs";
  std::string output_path = "resultado.txt";
  bool loaded = false;

  while (true) {
    PrintMenu();
    int op = 0;
    std::cout << "Opcion: ";
    if (!(std::cin >> op)) return EXIT_FAILURE;

    if (op == 1) {
      std::cout << "Ruta del fichero: ";
      std::cin >> input_path;
      loaded = g.LoadFromFile(input_path);
      if (!loaded) std::cout << "No se pudo cargar el grafo.\n";
      else std::cout << "Grafo cargado: n=" << g.NumVertices()
                     << ", m=" << g.NumEdges() << "\n";
    } else if (op == 2) {
      if (!loaded) { std::cout << "Cargue primero el grafo.\n"; continue; }
      std::cout << "Origen (1.." << g.NumVertices() << "): ";
      std::cin >> origin;
      std::cout << "Destino (1.." << g.NumVertices() << "): ";
      std::cin >> dest;
    } else if (op == 3) {
      std::cout << "Estrategia (bfs/dfs): ";
      std::cin >> strategy;
    } else if (op == 4) {
      if (!loaded || origin < 1 || dest < 1) {
        std::cout << "Falta cargar grafo o definir origen/destino.\n";
        continue;
      }
      std::cout << "Fichero de salida (por defecto resultado.txt): ";
      std::cin >> output_path;
      Strategy st = StrategyFromString(strategy);
      SearchResult res = UninformedSearch::Run(g, origin, dest, st);
      if (res.found) res.total_cost = UninformedSearch::ComputePathCost(g, res.path);
      DumpReport(g, origin, dest, res, output_path, st);
    } else if (op == 5) {
      break;
    } else {
      std::cout << "Opcion invalida.\n";
    }
  }

  return EXIT_SUCCESS;
}
