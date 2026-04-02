
#include "convergence.h"
#include "fitness.h"
#include "random.h"


FamousTree::FamousTree(std::unique_ptr<Node> tree_, double fitness_, size_t generation_, size_t flatAddress_) {
    tree = tree_->clone();
    fitness = fitness_;
    generation = generation_;
    flatAddress = flatAddress_;
}


bool HallOfFame::tryInsert(NodePtr tree, double fit, size_t gen, size_t flatAddr, const Dataset& X) {
    const size_t nSample = std::min<size_t>(50, X.size());
    const size_t step = std::max<size_t>(1, X.size() / nSample);
    std::vector<double> fp(nSample);
    for (size_t i = 0; i < nSample; i++)
        fp[i] = tree->eval(X[i * step]);

    auto correlation = [&](const std::vector<double>& a, const std::vector<double>& b) {
        double ma = 0, mb = 0;
        for (size_t i = 0; i < nSample; i++) {
            ma += a[i];
            mb += b[i];
        }
        ma /= (double)nSample;
        mb /= (double)nSample;
        double num = 0, da = 0, db = 0;
        for (size_t i = 0; i < nSample; i++) {
            num += (a[i] - ma) * (b[i] - mb);
            da += (a[i] - ma) * (a[i] - ma);
            db += (b[i] - mb) * (b[i] - mb);
        }
        double denom = std::sqrt(da * db);
        return denom < 1e-12 ? 1.0 : std::abs(num / denom);
    };

    for (auto& entry : fames) {
        std::vector<double> efp(nSample);
        for (size_t i = 0; i < nSample; i++)
            efp[i] = entry.tree->eval(X[i * step]);
        if (correlation(fp, efp) > similarityThreshold) {
            if (fit < entry.fitness) {
                entry.tree = tree->clone();
                entry.fitness = fit;
                entry.generation = gen;
                entry.flatAddress = flatAddr;
            }
            return false;
        }
    }

    fames.emplace_back(tree->clone(), fit, gen, flatAddr);

    if (fames.size() > maxSize) {
        std::ranges::sort(fames, [](const FamousTree& a, const FamousTree& b){ return a.fitness < b.fitness; });
        fames.resize(maxSize);
    }
    return true;
}

std::vector<NodePtr> HallOfFame::sample() const {
    if (fames.empty()) return { };
    std::vector<NodePtr> out;
    out.reserve(injectSize);

    std::vector<size_t> indices(fames.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::ranges::shuffle(indices, rng());

    for (size_t i = 0; i < std::min(injectSize, fames.size()); i++)
        out.push_back(fames[indices[i]].tree->clone());
    return out;
}

void ConvergenceTracker::update(const ConvergenceIndicators& convInd, double bestFit) {
    bestFitnessHistory.push_back(bestFit);

    if (bestFitnessHistory.size() > convInd.window)
        bestFitnessHistory.erase(bestFitnessHistory.begin());
}

bool ConvergenceTracker::hasConverged(const ConvergenceIndicators& convInd, double cur_std, double cur_p) const {
    if (bestFitnessHistory.size() < convInd.window) return false;

    double improvement = bestFitnessHistory.front() - bestFitnessHistory.back();

    bool fit_stagnant = improvement < convInd.tol_fit;
    bool std_low = cur_std <= convInd.tol_std;
    bool pred_low = cur_p <= convInd.tol_p;

    return std_low && (fit_stagnant || pred_low);
}

void ConvergenceTracker::reset() {
    bestFitnessHistory.clear();
}
