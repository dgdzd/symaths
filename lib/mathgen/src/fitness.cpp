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
    return err + penalty * genRatio * tree->complexity();
}

void optimizeConstants(Node* tree, const Dataset& X, const std::vector<double>& Y, double lr, unsigned int steps) {
    std::vector<ConstNode*> constNodes;
    for (Node* n : tree->nodes())
        if (auto* c = dynamic_cast<ConstNode*>(n))
            constNodes.push_back(c); //false error
    if (constNodes.empty()) return;

    auto evalFit = [&]() { return fitness(tree, X, Y, 0.0, 0, 1); };

    for (unsigned int s = 0; s < steps; s++) {
        double step = lr / (1.0 + 0.1 * (double)s);

        for (ConstNode* c : constNodes) {
            double base = c->value;
            double f0   = evalFit();

            c->value = base + step;
            double f1 = evalFit();

            if (f1 < f0) continue;

            c->value = base - step;
            double f2 = evalFit();

            if (f2 < f0) continue;

            c->value = base;
        }
    }
}