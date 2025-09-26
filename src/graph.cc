#include "graph.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

bool Graph::LoadFromFile(const std::string& path) {
  n_ = 0;
  m_ = 0;
  adj_matrix_.clear();
  adj_list_.clear();

  std::ifstream in(path);
  if (!in) {
    std::cerr << "Error: no se pudo abrir el fichero: " << path << "\n";
    return false;
  }

  // Lee n
  if (!(in >> n_) || n_ == 0) {
    std::cerr << "Error: n invalido o ausente en el fichero.\n";
    return false;
  }

  // Inicializa matriz con -1, y 0 en diagonal
  adj_matrix_.assign(n_, std::vector<double>(n_, -1.0));
  for (std::size_t i = 0; i < n_; ++i) {
    adj_matrix_[i][i] = 0.0;
  }

  // Se esperan n(n-1)/2 valores en el orden (1,2), (1,3), ... (n-1,n)
  const std::size_t expected = (n_ * (n_ - 1)) / 2;
  double w = -1.0;
  std::size_t idx = 0;
  for (std::size_t i = 0; i < n_ - 1; ++i) {
    for (std::size_t j = i + 1; j < n_; ++j) {
      if (!(in >> w)) {
        std::cerr << "Error: faltan distancias; esperados " << expected
                  << " valores.\n";
        return false;
      }
      adj_matrix_[i][j] = w;
      adj_matrix_[j][i] = w;
      if (w >= 0.0) {
        ++m_;
      }
      ++idx;
    }
  }

  BuildAdjacencyList();
  return true;
}

void Graph::BuildAdjacencyList() {
  adj_list_.assign(n_ + 1, {});  // índices 1..n cómodos
  for (std::size_t i = 0; i < n_; ++i) {
    for (std::size_t j = 0; j < n_; ++j) {
      if (i != j && adj_matrix_[i][j] >= 0.0) {
        adj_list_[static_cast<int>(i) + 1].push_back(static_cast<int>(j) + 1);
      }
    }
  }
}

const std::vector<int>& Graph::Neighbors(int v) const {
  // v es 1..n
  return adj_list_[v];
}

double Graph::EdgeCost(int u, int v) const {
  // ids 1..n -> indices 0..n-1
  int i = u - 1;
  int j = v - 1;
  if (i < 0 || j < 0 || i >= static_cast<int>(n_) || j >= static_cast<int>(n_)) {
    return -1.0;
  }
  return adj_matrix_[i][j];
}
