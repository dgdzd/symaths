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

//A FAIRE:
//- CMA-ES + AJUSTEMENT AU 10^-i où i est choisit (en faisant une fonction à part)


class ModelManager {
public:
    std::vector<std::string> variables;
    size_t populationSize = 100;
    unsigned int maxDepth = 7;
    double penalty = 1e-5;
    double mutationProb = 0.4;
    std::tuple<double, double, double> probs = { 0.25, 0.25, 0.25 };//(const, var, binary)
    std::vector<NodePtr> population;
    Dataset X;
    std::vector<double> Y;
    Operators ops;
    unsigned int k = 7;

    ModelManager(std::vector<std::string> variables_ = {"x"}, size_t populationSize_ = 100, unsigned int maxDepth_ = 5, double penalty_ = 0.01, double mutationProb_ = 0.3,
        const std::tuple<double,double,double>& probs_ = {0.25, 0.25, 0.25}, unsigned int k_ = 7);
    ModelManager(ModelManager&&)  noexcept = default;
    ModelManager& operator=(ModelManager&&) = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    void updateData(Dataset x, std::vector<double> y);

    void initPopulation(BinaryMap binaryOperators, UnaryMap unaryOperators, UnaryMap extraUnaryOperators = {});
    void loadPopulation(std::vector<NodePtr> population_, BinaryMap binaryOperators, UnaryMap unaryOperators, UnaryMap extraUnaryOperators = {}, bool fillPop = false);
    std::vector<NodePtr> getPopulation(bool sortFitness = true);
    std::string getTree(size_t idx);

    std::vector<double> residuals(const Node* tree) const;
    std::pair<std::vector<double>, double> normalizedResiduals(const Node* tree) const;
    static std::vector<double> denormalizePrediction(const std::vector<double>& res, double scale) ;


    void fit(size_t generations = 10, size_t maxPop = 100, size_t eliteSize = 10, size_t newbornSize = 10, double lr = 0.05, unsigned int cstOptiStep = 50,
        bool debug = false, unsigned int timeoutSeconds = 60, const std::function<bool(double)>& earlyStopCondition = nullptr);

private:
    double evalFitness(const Node* tree, size_t gen, size_t maxGen) const;
    [[nodiscard]] const Node* tournamentSelect(size_t gen, size_t maxGen) const;
};


#endif