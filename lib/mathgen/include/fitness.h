#ifndef SYMATHS_FITNESS_H
#define SYMATHS_FITNESS_H

#include "node.h"
#include <vector>

using Dataset = std::vector<Sample>;

//MAE
double fitness(Node* tree, const Dataset& X, const std::vector<double>& Y, double penalty, size_t gen, size_t maxGen);

// fine tune algo
void optimizeConstants(Node* tree, const Dataset& X, const std::vector<double>& Y, double lr = 0.05, unsigned int steps = 50);

#endif