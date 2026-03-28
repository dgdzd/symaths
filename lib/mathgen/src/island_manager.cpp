
#include "island_manager.h"
#include "fitness.h"
#include "tree_utils.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <chrono>
#include <utility>

static Isle makeIsle(const IsleConfig& cfg) {
    Isle isle(cfg.variables, cfg.populationSize, cfg.maxDepth, cfg.penalty, cfg.mutationProb, cfg.probs, cfg.k);
    isle.initPopulation(cfg.binaryOps, cfg.unaryOps);
    return isle;
}

IslandManager::IslandManager(const std::vector<GroupConfig>& groupConfigs, HallOfFame hallOfFame_, unsigned int migrationInterval_, unsigned int migrantCount_,
    double eliteFraction_) : migrationInterval(migrationInterval_), migrantCount(migrantCount_), eliteFraction(eliteFraction_), hallOfFame(std::move(hallOfFame_)) {

    if (groupConfigs.empty())
        throw std::invalid_argument("group config empty");
    if (eliteFraction_ < 0.0 || eliteFraction_ > 1.0)
        throw std::invalid_argument("incorrect eliteFraction");

    for (size_t gi = 0; gi < groupConfigs.size(); gi++) {
        const GroupConfig& g_cfg = groupConfigs[gi];

        double intra = g_cfg.intraSubgroupProb;
        double inter = g_cfg.interSubgroupProb;
        if (intra + inter > 1.0 + 1e-9)
            throw std::invalid_argument("group " + std::to_string(gi) + ": intraSubgroupProb + interSubgroupProb must be <= 1.0");

        Group group;
        group.intraSubgroupProb = intra;
        group.interSubgroupProb = inter;

        for (size_t si = 0; si < g_cfg.subgroups.size(); si++) {
            const SubGroupConfig& scfg = g_cfg.subgroups[si];
            const IsleConfig& isleCfg = scfg.isleOverride.has_value() ? *scfg.isleOverride : g_cfg.isleDefaults;
            SubGroup sg;
            for (size_t ii = 0; ii < scfg.numIsles; ii++) {
                sg.isles.emplace_back(std::move(makeIsle(isleCfg)));
                flatAddresses_.push_back({ gi, si, ii });
            }
            group.subgroups.push_back(std::move(sg));
        }
        groups_.push_back(std::move(group));
    }

    if (flatAddresses_.empty())
        throw std::invalid_argument("IslandManager: no isles were created.");
}

void IslandManager::updateData(const Dataset& X, const std::vector<double>& Y) {
    for (auto& addr : flatAddresses_)
        isleAt(addr).updateData(X, Y);
}


Isle& IslandManager::isleAt(const IsleAddress& a) {
    return groups_[a.group].subgroups[a.subgroup].isles[a.isle];
}
const Isle& IslandManager::isleAt(const IsleAddress& a) const {
    return groups_[a.group].subgroups[a.subgroup].isles[a.isle];
}

IslandManager::IsleAddress

IslandManager::pickDestination(const IsleAddress& src) const {
    const Group& srcGroup = groups_[src.group];
    double r = randDouble(0.0, 1.0);

    auto pickFromSubgroup = [&](size_t gi, size_t si) -> IsleAddress {
        const SubGroup& sg = groups_[gi].subgroups[si];
        std::vector<size_t> candidates;
        for (size_t ii = 0; ii < sg.isles.size(); ii++)
            if (gi != src.group || si != src.subgroup || ii != src.isle)
                candidates.push_back(ii);
        if (candidates.empty()) return src;
        return { gi, si, randChoice(candidates) };
    };

    if (r < srcGroup.intraSubgroupProb)
        return pickFromSubgroup(src.group, src.subgroup);

    if (r < srcGroup.intraSubgroupProb + srcGroup.interSubgroupProb) {
        const size_t numSG = srcGroup.subgroups.size();
        if (numSG <= 1)
            return pickFromSubgroup(src.group, src.subgroup);

        std::vector<size_t> sgCandidates;
        for (size_t si = 0; si < numSG; si++)
            if (si != src.subgroup) sgCandidates.push_back(si);
        size_t destSG = randChoice(sgCandidates);
        return pickFromSubgroup(src.group, destSG);
    }

    const size_t numGroups = groups_.size();
    if (numGroups <= 1)
        return pickFromSubgroup(src.group, src.subgroup);

    std::vector<size_t> gCandidates;
    for (size_t gi = 0; gi < numGroups; gi++)
        if (gi != src.group) gCandidates.push_back(gi);
    size_t destG = randChoice(gCandidates);
    size_t destSG = randInt(0, static_cast<int>(groups_[destG].subgroups.size()) - 1);
    return pickFromSubgroup(destG, destSG);
}

std::vector<NodePtr> IslandManager::collectMigrants(const Isle& src) const {
    std::vector<NodePtr> migrants;
    migrants.reserve(migrantCount);

    const auto& pop = src.population;
    if (pop.empty()) return migrants;

    auto nElites  = static_cast<unsigned int>(std::round(migrantCount * eliteFraction));
    unsigned int nNormals = migrantCount - nElites;

    unsigned int eliteEnd = std::min(nElites, static_cast<unsigned int>(pop.size()));
    for (unsigned int i = 0; i < eliteEnd; i++)
        migrants.push_back(pop[i]->clone());

    unsigned int normalStart = eliteEnd;
    auto normalEnd = static_cast<unsigned int>(pop.size());
    if (normalStart < normalEnd) {
        for (unsigned int i = 0; i < nNormals; i++) {
            int idx = randInt((int)normalStart, (int)normalEnd - 1);
            migrants.push_back(pop[idx]->clone());
        }
    }

    return migrants;
}


void IslandManager::injectMigrants(Isle& dest, std::vector<NodePtr> migrants) {
    if (migrants.empty() || dest.population.empty()) return;

    auto& pop = dest.population;
    size_t replaceStart = pop.size() > migrants.size() ? pop.size() - migrants.size() : 0;

    for (size_t i = 0; i < migrants.size(); i++) {
        size_t idx = replaceStart + i;
        if (idx < pop.size())
            pop[idx] = std::move(migrants[i]);
        else
            pop.push_back(std::move(migrants[i]));
    }
}

//replace any operator not found in destOps with a random valid operator from destOps
NodePtr IslandManager::adaptTree(NodePtr tree, const Operators& destOps) {
    if (!tree) return tree;

    if (auto* u = dynamic_cast<UnaryNode*>(tree.get())) {
        u->child = adaptTree(std::move(u->child), destOps);
        if (destOps.unary.find(u->name) == destOps.unary.end()) {
            const std::string& newName = randKey(destOps.unary);
            u->name = newName;
            u->func = destOps.unary.at(newName);
        }
    }
    else if (auto* b = dynamic_cast<BinaryNode*>(tree.get())) {
        b->left  = adaptTree(std::move(b->left),  destOps);
        b->right = adaptTree(std::move(b->right), destOps);
        if (destOps.binary.find(b->op) == destOps.binary.end()) {
            const std::string& newOp = randKey(destOps.binary);
            b->op   = newOp;
            b->func = destOps.binary.at(newOp);
        }
    }
    return tree;
}

void IslandManager::runMigration() {
    struct MigrationEvent {
        IsleAddress dest;
        std::vector<NodePtr> migrants;
    };

    std::vector<MigrationEvent> events;
    events.reserve(flatAddresses_.size());

    for (const auto& srcAddr : flatAddresses_) {
        const Isle& src = isleAt(srcAddr);
        IsleAddress destAddr = pickDestination(srcAddr);
        auto migrants = collectMigrants(src);
        events.push_back({ destAddr, std::move(migrants) });
    }

    for (auto& ev : events)
        injectMigrants(isleAt(ev.dest), std::move(ev.migrants));
}

void IslandManager::run(unsigned int totalGenerations, size_t maxPop, size_t eliteSize, size_t newbornSize, CMAESConfig cmaesCfg, bool debug,
    unsigned int timeoutSeconds, const std::function<bool(double)>& earlyStop) {

    using Clock = std::chrono::steady_clock;
    auto timeStart = Clock::now();

    const unsigned int numCycles = (totalGenerations + migrationInterval - 1) / migrationInterval;

    for (unsigned int cycle = 0; cycle < numCycles; cycle++) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - timeStart).count();
        if (elapsed >= timeoutSeconds)
            return;

        std::vector<std::thread> threads;
        threads.reserve(flatAddresses_.size());

        for (const auto& addr : flatAddresses_) {
            threads.emplace_back([&, addr]() {
                Isle& isle = isleAt(addr);
                isle.fit(migrationInterval, maxPop, eliteSize, newbornSize, cmaesCfg, false, timeoutSeconds, nullptr);
            });
        }
        for (auto& t : threads) t.join();

        if (debug) {
            unsigned int gensRun = (cycle + 1) * migrationInterval;
            std::cout << "=== Migration cycle: " << cycle
                      << "  (Gen " << gensRun << ") ===\n";

            for (size_t gi = 0; gi < groups_.size(); gi++) {
                for (size_t si = 0; si < groups_[gi].subgroups.size(); si++) {
                    for (size_t ii = 0; ii < groups_[gi].subgroups[si].isles.size(); ii++) {
                        const Isle& isle = groups_[gi].subgroups[si].isles[ii];
                        if (isle.population.empty()) continue;
                        const Node* best = isle.population[0].get();
                        double f = fitness(const_cast<Node*>(best), isle.X, isle.Y, isle.penalty, 0, 1);
                        std::cout << "  G" << gi << ".SG" << si << ".I" << ii << "  best fitness: " << std::fixed << std::setprecision(4) << f << "  expr: " << printTree(best) << "\n";
                    }
                }
            }
            std::cout << "-----------------------------------------\n";
        }

        if (earlyStop) {
            NodePtr best = bestTree();
            double f = fitness(best.get(), isleAt(flatAddresses_[0]).X, isleAt(flatAddresses_[0]).Y, 0.0, 0, 1);
            if (earlyStop(f))
                return;
        }

        runMigration();
    }
}

NodePtr IslandManager::bestTree() const {
    const Isle* best = &bestIsle();
    if (best->population.empty()) return nullptr;
    return best->population[0]->clone();
}

const Isle& IslandManager::bestIsle() const {
    const Isle* bestIslePtr = nullptr;
    double bestF = std::numeric_limits<double>::max();

    for (const auto& addr : flatAddresses_) {
        const Isle& isle = isleAt(addr);
        if (isle.population.empty()) continue;
        double f = fitness(const_cast<Node*>(isle.population[0].get()), isle.X, isle.Y, isle.penalty, 0, 1);
        if (f < bestF) {
            bestF = f;
            bestIslePtr = &isle;
        }
    }
    if (!bestIslePtr)
        throw std::runtime_error("no isle has a non-empty population.");
    return *bestIslePtr;
}

std::vector<const Isle*> IslandManager::allIsles() const {
    std::vector<const Isle*> result;
    result.reserve(flatAddresses_.size()); //false error
    for (const auto& addr : flatAddresses_)
        result.push_back(&isleAt(addr));
    return result;
}