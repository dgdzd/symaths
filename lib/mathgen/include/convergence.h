#ifndef SYMATHS_CONVERGENCE_H
#define SYMATHS_CONVERGENCE_H

#include "node.h"
#include "fitness.h"

struct FamousTree {
    std::unique_ptr<Node> tree = nullptr;
    double fitness = 0.0;
    size_t generation = 0; //discover generation
    size_t flatAddress = 0;

    FamousTree() = default;

    FamousTree(std::unique_ptr<Node> tree_, double fitness_, size_t generation_, size_t flatAddress_);
};

struct HallOfFame {
    std::vector<FamousTree> fames;
    size_t maxSize = 10;
    size_t injectSize = 2;

    bool tryInsert(NodePtr tree, double fit, size_t gen, size_t flatAddr, const Dataset& X);

    [[nodiscard]] std::vector<NodePtr> sample() const;
};


struct ConvergenceIndicators {
    double tol_std = 0.01;
    double tol_fit = 1e-4;
    double tol_p = 0.01;

    size_t window = 10;
};

struct ConvergenceTracker {
    std::vector<double> bestFitnessHistory;

    void update(const ConvergenceIndicators& convInd, double bestFit);

    [[nodiscard]] bool hasConverged(const ConvergenceIndicators& convInd, double cur_std, double cur_p) const;

    void reset();
};

#endif