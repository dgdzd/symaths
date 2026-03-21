#include "model_manager.h"
#include "random.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <stdexcept>
#include <utility>

ModelManager::ModelManager(std::vector<std::string> variables_, size_t populationSize_, unsigned int maxDepth_, double penalty_, double mutationProb_, const std::tuple<double,double,double>& probs_) :
    variables(std::move(variables_)) , populationSize(populationSize_) , maxDepth(maxDepth_) , penalty(penalty_) , mutationProb(mutationProb_) , probs(probs_) {

    seedRng(0);
}

void ModelManager::updateData(Dataset x, std::vector<double> y) {
    X = std::move(x);
    Y = std::move(y);
}

void ModelManager::initPopulation(BinaryMap binaryOperators, UnaryMap  unaryOperators, UnaryMap  extraUnaryOperators){
    for (auto& [k, v] : extraUnaryOperators)
        unaryOperators[k] = v;

    if (binaryOperators.empty() || unaryOperators.empty())
        ops = Operators();
    else
        ops = Operators(std::move(binaryOperators), std::move(unaryOperators));
    population.clear();

    while (static_cast<int>(population.size()) < populationSize) {
        auto tree = randomTree(static_cast<int>(maxDepth), variables, probs, ops.unary, ops.binary);
        if (!isMostlyConstants(tree.get()))
            population.push_back(std::move(tree));
    }
}

void ModelManager::loadPopulation(std::vector<NodePtr> population_, BinaryMap binaryOperators, UnaryMap unaryOperators, UnaryMap extraUnaryOperators) {
    for (auto& [k, v] : extraUnaryOperators)
        unaryOperators[k] = v;

    if (binaryOperators.empty() || unaryOperators.empty())
        ops = Operators();
    else
        ops = Operators(std::move(binaryOperators), std::move(unaryOperators));

    population = std::move(population_);
}

std::vector<NodePtr> ModelManager::getPopulation(bool sortFitness) {
    std::ranges::sort(population, [&](const NodePtr& a, const NodePtr& b) {
        return evalFitness(a.get(), 0, 0) < evalFitness(b.get(), 0, 0);
    });
    return std::move(population);
}


std::vector<double> ModelManager::residuals(const Node* tree) const {
    std::vector<double> res;
    res.reserve(Y.size());
    for (std::size_t i = 0; i < Y.size(); ++i)
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

ERRORCODE ModelManager::fit(size_t generations, size_t maxPop, size_t eliteSize, bool debug, unsigned int timeoutSeconds, const std::function<bool(double)>& earlyStopCondition) {
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

        if (shouldStop)
            return ERRORCODE::SHOULD_STOP;

        if (debug) std::cout << "Gen " << gen << "\n";
        size_t printCount = std::min(eliteSize, population.size());
        for (size_t i = 0; i < printCount; ++i) {
            Node* tree = population[i].get();
            optimizeConstants(tree, X, Y, 0.05, 50); //A CHANGER
            double f = evalFitness(tree, gen, generations);
            if (debug) std::cout << "  expr " << std::setw(2) << (i + 1) << ": "
                                 << printTree(tree)
                                 << " | fitness: " << std::fixed << std::setprecision(4) << f
                                 << "\n";
        }
        if (debug) std::cout << "----------------\n";

        std::vector<NodePtr> newPop;
        newPop.reserve(maxPop);
        for (int i = 0; i < eliteSize && i < static_cast<int>(population.size()); ++i)
            newPop.push_back(population[i]->clone());

        // Gene pool for tournament: top 50 (or full population if smaller) A CHANGER
        int poolSize = std::min(50, static_cast<int>(population.size()));

        while (newPop.size() < maxPop) {
            int idx1 = randInt(0, poolSize - 1);
            int idx2;
            do { idx2 = randInt(0, poolSize - 1); } while (idx2 == idx1);

            NodePtr child = crossover(population[idx1].get(), population[idx2].get());
            child = prune(std::move(child));
            child = mutateSubtree(std::move(child), static_cast<int>(maxDepth), variables, mutationProb, probs, ops.unary, ops.binary);
            mutateConstants(child.get());
            mutateOperator(child.get(), mutationProb, ops.binary, ops.unary);
            child = prune(std::move(child));

            if (!isMostlyConstants(child.get()))
                newPop.push_back(std::move(child));
        }

        population = std::move(newPop);
    }

    return ERRORCODE::OK;
}
