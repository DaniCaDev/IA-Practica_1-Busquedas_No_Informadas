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
    if (a == "--input" && i + 1 < argc) opts->input_path = argv[++i];
    else if (a == "--origin" && i + 1 < argc) opts->origin = std::stoi(argv[++i]);
    else if (a == "--dest" && i + 1 < argc) opts->dest = std::stoi(argv[++i]);
    else if (a == "--strategy" && i + 1 < argc) opts->strategy = argv[++i];
    else if (a == "--output" && i + 1 < argc) opts->output_path = argv[++i];
    else {
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

void DumpReport(const Graph& g, int origin, int dest, const SearchResult& r,
                const std::string& out_path) {
  std::ofstream out(out_path);
  if (!out) {
    std::cerr << "Error: no se pudo abrir el fichero de salida: " << out_path << "\n";
    return;
  }
  out << "Numero de nodos del grafo: " << g.NumVertices() << "\n";
  out << "Numero de aristas del grafo: " << g.NumEdges() << "\n\n";
  out << "Vertice origen: " << origin << "\n";
  out << "Vertice destino: " << dest << "\n\n";

  const std::size_t steps = std::min(r.frontier_steps.size(), r.visited_steps.size());
  for (std::size_t i = 0; i < steps; ++i) {
    out << "Iteracion " << (i + 1) << "\n";
    out << "Nodos generados: ";
    if (r.frontier_steps[i].empty()) {
      out << "-\n";
    } else {
      for (std::size_t j = 0; j < r.frontier_steps[i].size(); ++j) {
        if (j) out << ", ";
        out << r.frontier_steps[i][j];
      }
      out << "\n";
    }
    out << "Nodos inspeccionados: ";
    if (r.visited_steps[i].empty()) {
      out << "-\n\n";
    } else {
      for (std::size_t j = 0; j < r.visited_steps[i].size(); ++j) {
        if (j) out << ", ";
        out << r.visited_steps[i][j];
      }
      out << "\n\n";
    }
  }

  if (r.found) {
    out << "Camino: ";
    for (std::size_t i = 0; i < r.path.size(); ++i) {
      if (i) out << " - ";
      out << r.path[i];
    }
    out << "\n";
    out << "Costo: " << std::fixed << std::setprecision(2) << r.total_cost << "\n";
  } else {
    out << "Camino: no encontrado\n";
  }
  std::cout << "Informe generado en: " << out_path << "\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  CliOptions opts;
  if (!ParseArgs(argc, argv, &opts)) {
    std::cerr
        << "Uso:\n  " << argv[0]
        << " --input datos.txt --origin 1 --dest 4 --strategy bfs|dfs --output resultado.txt\n"
        << "O lanza sin argumentos para modo interactivo.\n";
    return EXIT_FAILURE;
  }

  Graph g;
  if (!opts.input_path.empty() && g.LoadFromFile(opts.input_path)) {
    if (opts.origin.has_value() && opts.dest.has_value()) {
      Strategy st = StrategyFromString(opts.strategy);
      SearchResult res = UninformedSearch::Run(g, *opts.origin, *opts.dest, st);
      if (res.found) {
        res.total_cost = UninformedSearch::ComputePathCost(g, res.path);
      }
      DumpReport(g, *opts.origin, *opts.dest, res, opts.output_path);
      return EXIT_SUCCESS;
    }
  }

  // --- MODO INTERACTIVO (menú sencillo) ---
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
      DumpReport(g, origin, dest, res, output_path);
    } else if (op == 5) {
      break;
    } else {
      std::cout << "Opcion invalida.\n";
    }
  }
  return EXIT_SUCCESS;
}
