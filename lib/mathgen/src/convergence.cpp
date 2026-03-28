
#include "convergence.h"
#include "fitness.h"


FamousTree::FamousTree(std::unique_ptr<Node> tree_, double fitness_, size_t generation_, size_t flatAddress_) {
    tree = tree_->clone();
    fitness = fitness_;
    generation = generation_;
    flatAddress = flatAddress_;
}


bool ConvergenceIndicators::hasConverged(double cur_std, double cur_fit, double cur_p) {
    return (cur_std <= tol_std) && ((cur_fit <= tol_fit) || (cur_p <= tol_p));
}