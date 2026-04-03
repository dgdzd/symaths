#ifndef SYMATHS_ISLAND_H
#define SYMATHS_ISLAND_H

#include "model_manager.h"
#include "operators.h"
#include "convergence.h"

#include <optional>
#include <string>
#include <tuple>


using Isle = ModelManager;

struct IsleConfig {
    std::vector<std::string> variables = {"x"};
    unsigned int maxDepth = 6;
    double penalty = 1e-4;
    double mutationProb = 0.4;
    std::tuple<double, double, double> probs = { 0.15, 0.25, 0.25 };
    size_t populationSize = 200;
    unsigned int k = 7;
    BinaryMap binaryOps;
    UnaryMap unaryOps;

    IsleConfig() = default;
};

struct SubGroupConfig {
    size_t numIsles = 2;
    std::optional<IsleConfig> isleOverride; // nullopt = group default
};

struct GroupConfig {
    IsleConfig isleDefaults; // base params for every isle in this group
    std::vector<SubGroupConfig> subgroups;

    ConvergenceIndicators convergence;
    size_t backupSize = 40;
    bool isPrimary = false;
    //interGroupProb is 1 - intra - inter
    double intraSubgroupProb = 0.70;  // migrate to isle in same subgroup
    double interSubgroupProb = 0.20;  // migrate to isle in another subgroup of this group

    double backupEliteThreshold = 0.99;
    double backupDiverseThreshold = 0.95;
};

#endif