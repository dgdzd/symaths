#ifndef SYMATHS_FITNESS_H
#define SYMATHS_FITNESS_H

#include "node.h"
#include "cma-es/cma_es.h"

#include <vector>
#include <functional>

using Dataset = std::vector<Sample>;

//MAE
double fitness(Node* tree, const Dataset& X, const std::vector<double>& Y, double penalty, size_t gen, size_t maxGen);

// CMA-ES algo or optiBFGS
void optimizeConstants(Node* tree, const Dataset& X, const std::vector<double>& Y, const CMAESConfig& cfg = { }, size_t k = 8);

void optiBFGS(std::vector<double>& x, const std::function<double(const std::vector<double>& vals)>& applyAndEval, size_t n, double tol = 1e-6, size_t max_iter = 100);

#endif