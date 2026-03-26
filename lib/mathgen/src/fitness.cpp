#include "fitness.h"

#include <stdexcept>

double fitness(Node* tree, const Dataset& X, const std::vector<double>& Y, double penalty, size_t gen,size_t maxGen) {
    if (X.size() != Y.size())
        throw std::runtime_error("fitness: X and Y size mismatch");

    double err = 0.0;
    const std::size_t n = Y.size();

    for (std::size_t i = 0; i < n; i++) {
        double pred = tree->eval(X[i]);
        err += std::abs(pred - Y[i]);
    }
    err /= static_cast<double>(n);

    double genRatio = (maxGen != 0) ? static_cast<double>(gen) / static_cast<double>(maxGen) : 0.0;
    double f_fitness = err + penalty * genRatio * tree->complexity();
    return (std::isnan(f_fitness) || std::isinf(f_fitness)) ? 1e20 : f_fitness;
}

void optimizeConstants(Node* tree, const Dataset& X, const std::vector<double>& Y, double lr, unsigned int steps) {



}