#include "../../../lib/mathgen//include/model_manager.h"
#include "../../../lib/mathgen//include/island_manager.h"

#include <cmath>
#include <iostream>
#include <optional>

double sum0(double n) {
    double out = 0.0;
    for (int k = 0; k < (int)n; k++) {
        out += std::sin(k)/k;
    }
    return out;
}

void test_model_manager() {
    //1. Define operators
    BinaryMap binaryFunc = {
        { "+", [](double a, double b){ return a + b; } },
        { "-", [](double a, double b){ return a - b; } },
        { "*", [](double a, double b){ return a * b; } },
        { "/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; } },
    };
    UnaryMap unaryFunc = {
        { "sin", [](double x){ return std::sin(x); } },
        { "cos", [](double x) { return std::cos(x); } },
        { "square", [](double x){ return x * x; } },
        { "exp", [](double x){ return std::abs(x) < 100.0 ? std::exp(x) : 0.0; } },
        { "log", [](double x) { return x == 0.0 ? 0.0 : std::log(std::abs(x)); } }
    };

    //2. Build dataset
    Dataset X;
    std::vector<double> Y;
    double from = -5;
    double to = 10;
    double precision = 0.05;
    for (int i = static_cast<int>(from / precision); i < static_cast<int>(to / precision); i++) {
        double x_v = i * precision;
        X.push_back({ { "x", x_v } });
        Y.push_back(sum0(x_v));
    }

    //3. Configure and run
    ModelManager manager(
        { "x" }, //variable names
        2000, //population size
        10, //max tree depth
        1e-6, //complexity penalty
        0.4, //mutation probability
        { 0.15, 0.25, 0.25 }, //(const_prob, var_prob, binary_prob)
        7 //Tournament k
    );
    manager.initPopulation(binaryFunc, unaryFunc);
    manager.updateData(X, Y);
    manager.fit(
        /*generations*/ 100,
        /*maxPop*/ 2000,
        /*eliteSize*/ 100,
        /*newbornSize*/ 200,
        /*config for cma-es*/ { },
        /*constant size for cma-es threshold*/ 8,
        /*debug*/ true,
        /*timeoutSecs*/ 3600,
        [](double fitness) { return fitness < 1e-3; }
    );

    std::cout << manager.getTree(0);
}

void test_island_manager() {
    BinaryMap binaryFunc = {
        { "+", [](double a, double b){ return a + b; } },
        { "-", [](double a, double b){ return a - b; } },
        { "*", [](double a, double b){ return a * b; } },
        { "/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; } },
    };

    UnaryMap unary = {
        { "sin", [](double x){ return std::sin(x); } },
        { "cos", [](double x){ return std::cos(x); } },
        { "square", [](double x){ return x * x; } },
        { "cube", [](double x){ return x * x * x; } },
    };

    // Dataset: \sin(x^{2}+\sin(x^{3}+\cos(x)))
    Dataset X;
    std::vector<double> Y;
    double from = -10;
    double to = 10;
    double precision = 0.05;
    for (int i = static_cast<int>(from / precision); i < static_cast<int>(to / precision); i++) {
        double xv = i * precision;
        X.push_back({ { "x", xv } });
        Y.push_back(std::sin(xv * xv + std::sin(xv * xv * xv + std::cos(xv))));
    }

    // Group 0
    IsleConfig baseCfg0;
    baseCfg0.variables = { "x" };
    baseCfg0.maxDepth = 12;
    baseCfg0.penalty = 1e-4;
    baseCfg0.mutationProb = 0.4;
    baseCfg0.probs = { 0.15, 0.25, 0.25 };
    baseCfg0.populationSize = 300;
    baseCfg0.k = 7;
    baseCfg0.binaryOps = binaryFunc;
    baseCfg0.unaryOps = unary;

    // SG 0.1 override: more explorative
    IsleConfig exploreCfg = baseCfg0;
    exploreCfg.mutationProb = 0.6;
    exploreCfg.maxDepth = 5;

    GroupConfig group0;
    group0.isleDefaults = baseCfg0;
    group0.intraSubgroupProb = 0.60;
    group0.interSubgroupProb = 0.25;
    group0.subgroups = {
        SubGroupConfig{ 3, std::nullopt }, // SG 0.0 — inherit baseCfg0
        SubGroupConfig{ 2, exploreCfg }, // SG 0.1 — explorative override
    };

    // Group 1
    IsleConfig baseCfg1 = baseCfg0;
    baseCfg1.unaryOps = unary;
    baseCfg1.maxDepth = 6;

    GroupConfig group1;
    group1.isleDefaults = baseCfg1;
    group1.intraSubgroupProb = 0.70;
    group1.interSubgroupProb = 0.20;
    group1.subgroups = {
        SubGroupConfig{ 3, std::nullopt }, // SG 1.0
    };

    CMAESConfig cfg;
    cfg.max_iter = 10;
    cfg.sigma0 = 1.0;

    // Run
    IslandManager manager({ group0, group1 }, { }, 10, 10, 0.6);
    manager.updateData(X, Y);
    manager.run(1000, 1000, 100, 100, cfg, 8, true, 3600);

    NodePtr best = manager.bestTree();
    if (best)
        std::cout << "\nBest Tree: " << printTree(best.get()) << "\n";

}

void test_visco_data()
{
    BinaryMap binaryFunc = {
        { "+", [](double a, double b){ return a + b; } },
        { "-", [](double a, double b){ return a - b; } },
        { "*", [](double a, double b){ return a * b; } },
        { "/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; } },
    };

    UnaryMap unaryFunc = {
        { "square", [](double x){ return x * x; } },
        { "cube", [](double x){ return x * x * x; } },
        { "sqrt", [](double x) { return sqrt(x); } },
        { "cbrt", [](double x) { return cbrt(x); } },
        { "exp", [](double x) { return exp(x); } },
        { "log", [](double x) { return x <= 0 ? 0.0 : log(x); } }
    };

    std::vector<std::unordered_map<std::string, double>> X = { };
    for (int x = 10; x <= 100; x += 10) {
        X.push_back( { { "T", x } } );
    }
    std::vector<double> Y = { 150, 78, 42, 23, 14, 9, 6.5, 4.8, 3.8, 3.1 };

    // Group 0
    IsleConfig baseCfg0;
    baseCfg0.variables = { "T" };
    baseCfg0.maxDepth = 7;
    baseCfg0.penalty = 1e-3;
    baseCfg0.mutationProb = 0.4;
    baseCfg0.probs = { 0.25, 0.25, 0.25 };
    baseCfg0.populationSize = 1000;
    baseCfg0.k = 7;
    baseCfg0.binaryOps = binaryFunc;
    baseCfg0.unaryOps = unaryFunc;

    // SG 0.1 override: more explorative
    IsleConfig exploreCfg = baseCfg0;
    exploreCfg.mutationProb = 0.8;

    // SG 0.0 override: more explorative
    IsleConfig closedCfg = baseCfg0;
    exploreCfg.mutationProb = 0.3;

    GroupConfig group0;
    group0.isleDefaults = baseCfg0;
    group0.intraSubgroupProb = 0.60;
    group0.interSubgroupProb = 0.25;
    group0.subgroups = {
        SubGroupConfig{ 3, closedCfg }, // SG 0.0 — closed override
        SubGroupConfig{ 2, exploreCfg }, // SG 0.1 — explorative override
    };

    //Group 1
    IsleConfig baseCfg1 = baseCfg0;

    GroupConfig group1;
    group1.isleDefaults = baseCfg1;
    group1.intraSubgroupProb = 0.70;
    group1.interSubgroupProb = 0.20;
    group1.subgroups = {
        SubGroupConfig{ 3, std::nullopt }, // SG 1.0
    };

    CMAESConfig cfg;
    cfg.max_iter = 50;
    cfg.sigma0 = 1.0;

    // Run
    IslandManager manager({ group0, group1 }, { }, 10, 10, 0.6);
    manager.updateData(X, Y);
    manager.run(1000, 1000, 100, 300, cfg, 8, true, 3600);

    NodePtr best = manager.bestTree();
    if (best)
        std::cout << "\nBest Tree: " << printTree(best.get()) << "\n";

    for (auto& fame : manager.hallOfFame.fames) {
        std::cout << printTree(fame.tree.get()) << "\n";
    }


}

double eval_(std::vector<double> c, const Dataset& X, const std::vector<double>& Y) {
    double er = 0;
    for (size_t i = 0; i < Y.size(); i++) {
        double x = X[i].at("x");
        er += abs(Y[i] - (x * (c[0]*x*x + c[1]*x + c[2]) / (c[3]*x*x + c[4]*x + c[5])));
    }
    return er / static_cast<double>(Y.size());
}

void test_cmaes() {
    //Dataset
    Dataset X = { {  {"x", 30 } }, {  {"x", 40 } }, {  {"x", 50 } }, {  {"x", 60 } }, {  {"x", 70 } } };
    std::vector<double> Y = { 47, 34, 25, 19, 15 };
    size_t n = 6; //number of parameters
    std::vector<double> x0 = { 1e-3, -1e-2, 50, 1e-2, -3, 50 }; //initial guesses (of size n)
    int max_gen = 10000;

    CMAESConfig cfg;
    cfg.max_iter = 1000;
    cfg.sigma0 = 1.0;

    CMAES optimizer(n, cfg);
    optimizer.set_mean(x0);

    while (!optimizer.converged() && optimizer.generation() < max_gen) {
        const auto& candidates = optimizer.ask(); // lambda candidates

        std::vector<double> scores(candidates.size());
        for (size_t k = 0; k < candidates.size(); k++)
            scores[k] = eval_(candidates[k], X, Y);

        optimizer.tell(scores); // update state
    }

    Vector best_coeffs = optimizer.best();
    double best_mre = optimizer.best_fitness();

    std::cout << "best MRE: " << best_mre;
    std::cout << "best coeffs: " << "\n";
    for (double best_coeff : best_coeffs)
        std::cout << best_coeff << ", ";
}

int main() {
    //A FAIRE
    //- (?) implémenter tolérance adaptive (suivant quelles variables, indicateurs, ... ?)
    //- (?) NOMBRES COMPLEXES EN OPTION (avec des templates ?)
    //- implémenter sommes et produits dans arbres

    //test_island_manager();
    //test_cmaes();
    test_visco_data();
}