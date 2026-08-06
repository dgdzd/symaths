#ifndef SYMATHS_MODEL_MANAGER_H
#define SYMATHS_MODEL_MANAGER_H

#include "node.h"
#include "operators.h"
#include "fitness.h"
#include "tree_utils.h"

#include <vector>
#include <string>
#include <functional>
#include <tuple>
#include <stdexcept>

#include "cma-es/params.h"

class ModelManager {
public:
    std::vector<std::string> variables;
    size_t populationSize = 100;
    unsigned int maxDepth = 7;
    double penalty = 1e-5;
    double mutationProb = 0.4;
    Probs probs = { 0.167, 0.167, 0.167, 0.167, 0.167 };//(const, var, binary, trinary, nary)
    Operators ops;
    unsigned int k = 7;

    std::vector<NodePtr> population;
    Dataset X;
    std::vector<double> Y;

    explicit ModelManager(std::vector<std::string> variables_ = { "x" }, size_t populationSize_ = 100, unsigned int maxDepth_ = 5, double penalty_ = 1e-2, double mutationProb_ = 0.3,
        Probs probs_ = { 0.167, 0.167, 0.167, 0.167, 0.167 }, unsigned int k_ = 7);
    ModelManager(ModelManager&&) noexcept = default;
    ModelManager& operator = (ModelManager&&) = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator = (const ModelManager&) = delete;

    void updateData(Dataset x, std::vector<double> y);

    void initPopulation(const UnaryMap& unaryOps,const BinaryMap& binaryOps, const TrinaryMap& trinaryOps, const NaryMap& naryOps);
    void loadPopulation(std::vector<NodePtr> population_, const UnaryMap& unaryOps, const BinaryMap& binaryOps, const TrinaryMap& trinaryOps, const NaryMap& naryOps,
        bool fillPop = false);

    std::vector<NodePtr> getPopulation(bool sortFitness = true);
    std::string getTree(size_t idx);

    void insertIndividual(NodePtr tree);
    void removeIndividual(size_t idx);
    void replaceIndividual(size_t idx, NodePtr tree);

    std::vector<double> residuals(const Node* tree) const;
    std::pair<std::vector<double>, double> normalizedResiduals(const Node* tree) const;
    static std::vector<double> denormalizePrediction(const std::vector<double>& res, double scale) ;

    void fit(size_t generations = 10, size_t maxPop = 100, size_t eliteSize = 10, size_t newbornSize = 10, CMAESConfig cfg = { }, size_t cmaesThreshold = 8,
        bool debug = false, unsigned int timeoutSeconds = 60, const std::function<bool(double)>& earlyStopCondition = nullptr);

private:
    double evalFitness(const Node* tree, size_t gen, size_t maxGen) const;
    [[nodiscard]] const Node* tournamentSelect(size_t gen, size_t maxGen) const;
};


#endif