#ifndef SYMATHS_CONVERGENCE_H
#define SYMATHS_CONVERGENCE_H

#include "node.h"
#include "fitness.h"
#include "tree_utils.h"

struct FamousTree {
    std::unique_ptr<Node> tree;
    double fitness;
    size_t generation; //discover generation
    size_t flatAddress;

    FamousTree(std::unique_ptr<Node> tree_, double fitness_, size_t generation_, size_t flatAddress_);
};

struct HallOfFame {
    std::vector<FamousTree> fames;
    size_t maxSize = 10;
    size_t injectSize = 2;
};


struct ConvergenceIndicators {
    double tol_std;
    double tol_fit;
    double tol_p;

    bool hasConverged(double cur_std, double cur_fit, double cur_p);
};

#endif