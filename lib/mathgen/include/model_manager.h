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
//- CHANGER LES STD::COUT POUR QUE RIEN NE SOIT AFFICHER SUR CMD ->MODE DEBUG SEULEMENT
//- MULTI THREADING (GPU ?)
//-POUVOIR CHANGER LA LR
//- CHANGER LA POOLSIZE
//- ISLAND CLASS/STRUCT
//- CMA-ES + AJUSTEMENT AU 10^-i où i est choisit (en faisant une fonction à part)

enum class ERRORCODE {
    OK,
    TIMEOUT,
    EARLY_CONDITION_REACHED,
    POPULATION_EMPTY,
    GEN_THRESHOLD_REACHED,
    ELITE_SIZE_OUT_OF_BOUNDS,
    DATASET_EMPTY,
    SHOULD_STOP
};

class ModelManager {
public:
    std::vector<std::string> variables;
    size_t populationSize;
    unsigned int maxDepth;
    double penalty;
    double mutationProb;
    std::tuple<double, double, double> probs;//(const, var, binary)
    std::vector<NodePtr> population;
    Dataset X;
    std::vector<double> Y;
    Operators ops;

    ModelManager(
        std::vector<std::string> variables_ = {"x"},
        size_t populationSize_ = 100,
        unsigned int maxDepth_ = 5,
        double penalty_ = 0.01,
        double mutationProb_ = 0.3,
        const std::tuple<double,double,double>& probs_ = {0.25, 0.25, 0.25}
    );

    void updateData(Dataset x, std::vector<double> y);

    void initPopulation(BinaryMap binaryOperators, UnaryMap unaryOperators, UnaryMap extraUnaryOperators = {});

    std::vector<double> residuals(const Node* tree) const;
    std::pair<std::vector<double>, double> normalizedResiduals(const Node* tree) const;
    static std::vector<double> denormalizePrediction(const std::vector<double>& res, double scale) ;

    //returns ok, timeout or stopCondition
    ERRORCODE fit(std::atomic<bool>& shouldStop, size_t generations = 10, size_t maxPop = 100, size_t eliteSize = 10, bool debug = false, unsigned int timeoutSeconds = 60, const std::function<bool(double)>& earlyStopCondition = nullptr);



private:
    double evalFitness(const Node* tree, size_t gen, size_t maxGen) const;
};


#endif