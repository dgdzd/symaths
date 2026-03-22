#include "model_manager.h"
#include "random.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <utility>

ModelManager::ModelManager(std::vector<std::string> variables_, size_t populationSize_, unsigned int maxDepth_, double penalty_,
    double mutationProb_, const std::tuple<double,double,double>& probs_, unsigned int k_) :
    variables(std::move(variables_)) , populationSize(populationSize_) , maxDepth(maxDepth_) , penalty(penalty_) , mutationProb(mutationProb_) , probs(probs_),
    k(k_) {

    seedRng(0);
}

void ModelManager::updateData(Dataset x, std::vector<double> y) {
    X = std::move(x);
    Y = std::move(y);
}

void ModelManager::initPopulation(BinaryMap binaryOperators, UnaryMap unaryOperators, UnaryMap extraUnaryOperators){
    for (auto& [k_, v] : extraUnaryOperators)
        unaryOperators[k_] = v;

    if (binaryOperators.empty() || unaryOperators.empty())
        ops = Operators();
    else
        ops = Operators(std::move(binaryOperators), std::move(unaryOperators));
    population.clear();

    while (static_cast<int>(population.size()) < populationSize) {
        auto tree = randomTree(maxDepth, variables, probs, ops.unary, ops.binary);
        if (!isMostlyConstants(tree.get()))
            population.push_back(std::move(tree));
    }
}

void ModelManager::loadPopulation(std::vector<NodePtr> population_, BinaryMap binaryOperators, UnaryMap unaryOperators, UnaryMap extraUnaryOperators, bool fillPop) {
    for (auto& [k_, v] : extraUnaryOperators)
        unaryOperators[k_] = v;

    if (binaryOperators.empty() || unaryOperators.empty())
        ops = Operators();
    else
        ops = Operators(std::move(binaryOperators), std::move(unaryOperators));

    population.clear();

    for (auto& tree : population_) {
        if (!isMostlyConstants(tree.get()))
            population.push_back(std::move(tree));
    }

    if (fillPop) {
        while (static_cast<int>(population.size()) < populationSize) {
            auto tree = randomTree(maxDepth, variables, probs, ops.unary, ops.binary);
            if (!isMostlyConstants(tree.get()))
                population.push_back(std::move(tree));
        }
    }
}

std::vector<NodePtr> ModelManager::getPopulation(bool sortFitness) {
    if (sortFitness) {
        std::ranges::sort(population, [&](const NodePtr& a, const NodePtr& b) {
            return evalFitness(a.get(), 0, 0) < evalFitness(b.get(), 0, 0);
        });
    }
    std::vector<NodePtr> out;
    out.reserve(population.size());
    for (const auto& t : population)
        out.push_back(t->clone());
    return out;
}


std::vector<double> ModelManager::residuals(const Node* tree) const {
    std::vector<double> res;
    res.reserve(Y.size());
    for (std::size_t i = 0; i < Y.size(); i++)
        res.push_back(Y[i] - tree->eval(X[i]));
    return res;
}

std::pair<std::vector<double>, double>
ModelManager::normalizedResiduals(const Node* tree) const {
    auto res = residuals(tree);
    double mean = std::accumulate(res.begin(), res.end(), 0.0) / static_cast<double>(res.size());
    double var  = 0.0;
    for (double r : res) var += (r - mean) * (r - mean);
    var /= static_cast<double>(res.size());
    double scale = std::sqrt(var);

    if (scale < 1e-8) return {res, 1.0};

    for (double& r : res) r = (r - mean) / scale;
    return {res, scale};
}

std::vector<double> ModelManager::denormalizePrediction(const std::vector<double>& res, double scale) {
    std::vector<double> out;
    out.reserve(res.size());
    for (double r : res) out.push_back(r * scale);
    return out;
}


double ModelManager::evalFitness(const Node* tree, size_t gen, size_t maxGen) const {
    return fitness(const_cast<Node*>(tree), X, Y, penalty, gen, maxGen);
}

const Node* ModelManager::tournamentSelect(size_t gen, size_t maxGen) const {
    const int n = static_cast<int>(population.size());
    const Node* best = nullptr;
    double bestFit = std::numeric_limits<double>::max();

    for (int i = 0; i < k; i++) {
        const Node* candidate = population[randInt(0, n - 1)].get();
        double f = evalFitness(candidate, gen, maxGen);
        if (f < bestFit) {
            bestFit = f;
            best = candidate;
        }
    }
    return best;
}

ERRORCODE ModelManager::fit(size_t generations, size_t maxPop, size_t eliteSize, size_t newbornSize, double lr, unsigned int cstOptiStep, bool debug, unsigned int timeoutSeconds,
    const std::function<bool(double)>& earlyStopCondition) {
    if (population.empty())
        return ERRORCODE::POPULATION_EMPTY;
    if (eliteSize < 1 || eliteSize >= maxPop)
       return ERRORCODE::ELITE_SIZE_OUT_OF_BOUNDS;
    if (X.empty() || Y.empty())
        return ERRORCODE::DATASET_EMPTY;

    using Clock = std::chrono::steady_clock;
    auto timeStart = Clock::now();

    for (size_t gen = 0; gen < generations; gen++) {
        std::ranges::sort(population, [&](const NodePtr& a, const NodePtr& b) {
            return evalFitness(a.get(), gen, generations) < evalFitness(b.get(), gen, generations);
        });

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - timeStart).count();
        if (elapsed >= timeoutSeconds)
            return ERRORCODE::TIMEOUT;

        if (earlyStopCondition) {
            double bestFit = evalFitness(population[0].get(), gen, generations);
            if (earlyStopCondition(bestFit))
                return ERRORCODE::EARLY_CONDITION_MET;
        }


        if (debug) std::cout << "Gen " << gen << "\n";
        size_t printCount = std::min(eliteSize, population.size());
        for (size_t i = 0; i < printCount; i++) {
            Node* tree = population[i].get();
            optimizeConstants(tree, X, Y, lr, cstOptiStep);
            double f = evalFitness(tree, gen, generations);
            if (debug) std::cout << "  expr " << std::setw(2) << (i + 1) << ": " << printTree(tree) << " | fitness: " << std::fixed << std::setprecision(4) << f << "\n";
        }
        if (debug) std::cout << "----------------\n";

        std::vector<NodePtr> newPop;
        newPop.reserve(maxPop);
        for (int i = 0; i < eliteSize && i < static_cast<int>(population.size()); i++)
            newPop.push_back(population[i]->clone());

        while (static_cast<int>(newPop.size()) < (maxPop - newbornSize - eliteSize)) {
            const Node* p1 = tournamentSelect(gen, generations);
            const Node* p2 = tournamentSelect(gen, generations);

            NodePtr child = crossover(p1, p2);
            child = prune(std::move(child));
            child = mutateSubtree(std::move(child), maxDepth, variables, mutationProb, probs, ops.unary, ops.binary);

            mutateConstants(child.get());
            mutateOperator(child.get(), mutationProb, ops.binary, ops.unary);
            child = prune(std::move(child));

            if (!isMostlyConstants(child.get()))
                newPop.push_back(std::move(child));
        }

        while (newPop.size() < maxPop) {
            auto newborn = randomTree(maxDepth, variables, probs, ops.unary, ops.binary);
            if (!isMostlyConstants(newborn.get()))
                newPop.push_back(std::move(newborn));
        }

        population = std::move(newPop);
    }

    return ERRORCODE::OK;
}
