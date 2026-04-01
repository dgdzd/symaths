#ifndef SYMATHS_ISLAND_MANAGER_H
#define SYMATHS_ISLAND_MANAGER_H

#include "island.h"
#include "node.h"
#include "convergence.h"

#include <vector>
#include <thread>
#include <functional>
#include <memory>

struct SubGroup {
    std::vector<Isle> isles;
};

struct Group {
    std::vector<SubGroup> subgroups;
    double intraSubgroupProb;
    double interSubgroupProb;
    ConvergenceIndicators convergence;

    std::vector<NodePtr> backup;
    size_t backupMaxSize = 40;
    bool isPrimary = false;
};

class IslandManager {
public:
    std::vector<Group> groups;
    HallOfFame hallOfFame;

    unsigned int migrationInterval;
    unsigned int migrantCount;
    double eliteFraction;

    IslandManager(const std::vector<GroupConfig>& groupConfigs, HallOfFame hallOfFame_, unsigned int migrationInterval_ = 5, unsigned int migrantSize_ = 5,
        double eliteFraction_ = 0.6);

    void updateData(const Dataset& X, const std::vector<double>& Y);

    void run(unsigned int totalGenerations, size_t maxPop, size_t eliteSize, size_t newbornSize, CMAESConfig cmaesCfg = { }, size_t cmaesThreshold = 8, bool debug = false,
        unsigned int timeoutSeconds = 3600, const std::function<bool(double)>& earlyStop = nullptr);

    [[nodiscard]] NodePtr bestTree() const;
    [[nodiscard]] const Isle& bestIsle() const;

    [[nodiscard]] std::vector<const Isle*> allIsles() const;
    [[nodiscard]] Isle& isleAt(size_t group, size_t subgroup, size_t isle);
    [[nodiscard]] const Isle& isleAt(size_t group, size_t subgroup, size_t isle) const;

    void clearBackup(size_t groupIdx);
    void injectIntoBackup(size_t groupIdx, NodePtr tree);
    void clearHallOfFame();

private:
    std::vector<ConvergenceTracker> convergenceTrackers;

    struct IsleAddress {
        size_t group;
        size_t subgroup;
        size_t isle;
    };
    std::vector<IsleAddress> flatAddresses_; //built once in constructor

    Isle& isleAt(const IsleAddress& a);
    [[nodiscard]] const Isle& isleAt(const IsleAddress& a) const;

    [[nodiscard]] IsleAddress pickDestination(const IsleAddress& src) const;

    [[nodiscard]] std::vector<NodePtr> collectMigrants(const Isle& src) const;

    void injectMigrants(Isle& dest, std::vector<NodePtr> migrants) ;

    static NodePtr adaptTree(NodePtr tree, const Operators& destOps);

    void runMigration();

    void refreshBackup();
    void refreshHallOfFame(size_t cycle);

    void injectHallOfFame();

    void handleConvergence(const IsleAddress& addr, size_t eliteSize);
};


#endif