<p align="center">
  <img width="700" alt="logo-transparent" src="https://github.com/user-attachments/assets/43898759-afd4-40ef-b5e7-ec9c967e668f" />
</p>

# symaths, an easy-to-use library

symaths is a mathematical tool developed by the same dudes who created [DataMorph](https://github.com/DataMorph-org/DataMorph). There are actually two projects in this repository : the core
library which is intented to be used by C++ developers, and the commandline tool which is for everyone.

# Table of contents

- [Using the core library](#using-the-core-library)
  - [Building](#building)
- A symbolic regression tool: [MathGen](#MathGen)

# Using the core library

To use the library, one must first ensure that they have CMake installed, as it will be useful for building and installing.

## Building


# MathGen

MathGen is a fast symbolic regression engine based on genetic programming. It evolves mathematical expression trees to fit a dataset, discovering formulas from it.

---

## How it works

The engine maintains a population of expression trees. Each tree is a mathematical formula built from constants, variables, and operators (unary and binary) . 
Over successive generations, trees are selected, crossed over, mutated, and pruned to minimize the error on the target dataset.
It features a built-in CMA-ES algorithm in order to optimize constants.

```
Population of trees
        │
        ▼
  Evaluate fitness (MAE + complexity penalty)
        │
        ▼
  Keep elite individuals
        │
        ▼
  Crossover + Mutation → new population
        │
        └──► repeat
```

The fitness function is:

```
fitness = MAE(tree, data) + penalty * (gen / maxGen) * complexity(tree)
```

The complexity penalty increases progressively over generations.

---

## Usage


### Quick start

```cpp
#include "model_manager.h"
#include <cmath>

int main() {
    //1. Define operators
    BinaryMap binaryFunc = {
        {"+", [](double a, double b){ return a + b; }},
        {"-", [](double a, double b){ return a - b; }},
        {"*", [](double a, double b){ return a * b; }},
        {"/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; }},
    };
    UnaryMap unaryFunc = {
        {"sin", [](double x){ return std::sin(x); }},
        {"square", [](double x){ return x * x; }},
    };

    //2. Build dataset  ->  y = x * sin(x)
    Dataset X;
    std::vector<double> Y;
    for (int i = -400; i < 400; i++) {
        double xv = i / 10.0;
        X.push_back({{"x", xv}});
        Y.push_back(xv * std::sin(xv));
    }

    //3. Configure and run
    ModelManager manager(
        {"x"}, //variable names
        400, //population size
        5, //max tree depth
        1e-5, //complexity penalty
        0.4, //mutation probability
        {0.1, 0.3, 0.3}, //(const_prob, var_prob, binary_prob)
        7
    );
    manager.initPopulation(binaryFunc, unaryFunc);
    manager.updateData(X, Y);
    manager.fit(
        /*generations*/ 10,
        /*maxPop*/ 400,
        /*eliteSize*/ 40,
        /*newbornSize*/ 30,
        /*lr*/ 0.05,
        /*cstOptiStep*/ 50,
        /*timeoutSecs*/ 90
    );
}
```

### `ModelManager`

```cpp
ModelManager(
    std::vector<std::string> variables,
    size_t populationSize = 100,
    unsigned int maxDepth = 5,
    double penalty = 0.01,
    double mutationProb = 0.3,
    std::tuple<double,double,double> probs = {0.25, 0.25, 0.25}, // (const, var, binary) — unary gets 1-sum
    unsigned int k = 7
);
```

| k | Behaviour |
|---|---|
| 1 | Pure random selection (no pressure) |
| 2–3 | low pressure, high diversity, slow convergence |
| 7 | Default for Symbolic Regression |
| 15–20 | Very high pressure, fast convergence (risk of premature convergence) |


| Method | Description |
|---|---|
| `initPopulation(binary, unary, extraUnary)` | Build the initial random population |
| `loadPopulation(population, binary, unary, extraUnary, fillPop)`| Load any population, fillPop fill population with new trees |
| `getPopulation(sortFitness)`| Return the current population (and sort it if wanted) |
| `updateData(X, Y)` | Set or replace the training dataset |
| `fit(generations, maxPop, eliteSize, timeout, earlyStop)` | Run the evolution loop |
| `residuals(tree)` | Raw residuals `y - ŷ` for a given tree |
| `normalizedResiduals(tree)` | Z-score normalized residuals + scale factor |

### `fit()` parameters

| Parameter | Type | Default | Description | Recommended value |
|---|---|---|---|---|
| `generations` | `size_t` | `10` | Number of evolutionary generations | 5 to 100 |
| `maxPop` | `size_t` | `100` | Maximum population size | 100 to 1000 |
| `eliteSize` | `size_t` | `10` | Number of top individuals carried over (unchanged) | 10 to 100 |
| `newbornSize`| `size_t` | `8` | Number of bottom individuals carried over (completely new) | 8 to 80 |
| `lr` | `double` | `0.05` | Learning rate | 0.01 to 0.1 |
| `cstOptiStep` | `50` | Constants optimization steps | 20 to 100 |
| `debug`| `bool` | `false`| Shows useful informations in the cmd | false |
| `timeoutSeconds` | `unsigned int` | `60` | Time limit | 60 to 3600 |
| `earlyStopCondition` | `std::function<bool(double)>` | `nullptr` | Callback receiving best fitness — return `true` to stop early | return fitness < 1e-6 |

Recommended values are not rules to follow.

### Probability tuple `probs = (const_prob, var_prob, binary_prob)`

Controls the shape of randomly generated trees. The unary probability is implicit: `1 - sum(probs)`.

| Value | Effect |
|---|---|
| High `const_prob` | More numeric constants, smaller trees |
| High `var_prob` | More variable references, shallower trees |
| High `binary_prob` | Wider, more complex trees (`+`, `*`, …) |
| Low all three | More unary nodes (`sin`, `square`, …) |

---

### Adding custom operators

Any `std::function<double(double)>` or `std::function<double(double,double)>` works:

```cpp
UnaryMap extra = {
    {"sigmoid", [](double x){ return 1.0 / (1.0 + std::exp(-x)); }},
    {"relu", [](double x){ return x > 0.0 ? x : 0.0; }},
};
manager.initPopulation(binaryFunc, unaryFunc, extra);
```

---

### Multi-variable regression

```cpp
// dataset for  z = x² + sin(y)
Dataset X;
std::vector<double> Y;
for (double xv = -3.0; xv <= 3.0; xv += 0.5)
    for (double yv = -3.0; yv <= 3.0; yv += 0.5) {
        X.push_back({{"x", xv}, {"y", yv}});
        Y.push_back(xv * xv + std::sin(yv));
    }

ModelManager manager({"x", "y"}, /*...*/ );
```

---

## Island Model

For harder problems, MathGen supports an Island Model: multiple independent populations (isles) evolve in parallel and periodically exchange individuals (migration). This prevents too fast convergence and allows different operator sets to let each isle to be specialized.

```
IslandManager
├── Group 0  (sin/cos operators)
│   ├── Subgroup 0.0 — Isle 0, Isle 1, Isle 2   (exploitative)
│   └── Subgroup 0.1 — Isle 3, Isle 4           (explorative, higher mutation)
└── Group 1  (exp/log operators)
    └── Subgroup 1.0 — Isle 5, Isle 6, Isle 7

Every n generations → migration between isles
```

### Migration

Each migration cycle, every isle sends a mix of elite and random individuals to another isle chosen probabilistically:

| Destination | Probability |
|---|---|
| Same subgroup | `intraSubgroupProb` (ex: 0.60) |
| Other subgroup, same group | `interSubgroupProb` (ex: 0.25) |
| Other group | implicit: `1 - intra - inter` (ex: 0.15) |

Incoming migrants **replace the worst individuals** on the destination isle. If migrants carry operators unknown to the destination isle, those operators are automatically replaced with a random valid one from the destination's operator set.

### Quick start

```cpp
#include "island_manager.h"
#include <cmath>

int main() {
    BinaryMap binaryFunc = {
        {"+", [](double a, double b){ return a + b; }},
        {"-", [](double a, double b){ return a - b; }},
        {"*", [](double a, double b){ return a * b; }},
        {"/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; }},
    };

    // Group 0 — trig focused
    UnaryMap unaryTrig = {
        {"sin", [](double x){ return std::sin(x); }},
        {"cos", [](double x){ return std::cos(x); }},
        {"square", [](double x){ return x * x; }},
        {"exp", [](double x){ return std::abs(x) < 10.0 ? std::exp(x) : 0.0; }},
    };

    // Group 1 — exp/log focused
    UnaryMap unaryExpLog = {
        {"exp", [](double x){ return std::abs(x) < 10.0 ? std::exp(x) : 0.0; }},
        {"log", [](double x){ return std::log(std::abs(x) + 1e-12); }},
        {"sin", [](double x){ return std::sin(x); }},
        {"sqrt", [](double x){ return std::sqrt(std::abs(x)); }},
    };

    Dataset X;
    std::vector<double> Y;
    for (int i = 0; i < 400; i++) {
        double xv = i * 0.05;
        X.push_back({{"x", xv}});
        Y.push_back(std::sin(xv) * std::exp(std::cos(xv * xv)));
    }

    // Group 0 config
    IsleConfig baseCfg0;
    baseCfg0.variables = { "x" };
    baseCfg0.maxDepth = 7;
    baseCfg0.penalty = 1e-4;
    baseCfg0.mutationProb = 0.4;
    baseCfg0.probs = { 0.15, 0.25, 0.25 };
    baseCfg0.populationSize = 300;
    baseCfg0.k = 7;
    baseCfg0.binaryOps = binaryFunc;
    baseCfg0.unaryOps = unaryTrig;

    // Subgroup 0.1 override — more explorative
    IsleConfig exploreCfg  = baseCfg0;
    exploreCfg.mutationProb = 0.6;
    exploreCfg.maxDepth = 5;

    GroupConfig group0;
    group0.isleDefaults = baseCfg0;
    group0.intraSubgroupProb = 0.60;
    group0.interSubgroupProb = 0.25;
    group0.subgroups = {
        SubGroupConfig{ 3, std::nullopt }, // SG 0.0 — 3 isles, inherits baseCfg0
        SubGroupConfig{ 2, exploreCfg },   // SG 0.1 — 2 isles, explorative override
    };

    // Group 1 config
    IsleConfig baseCfg1 = baseCfg0;
    baseCfg1.unaryOps = unaryExpLog;
    baseCfg1.maxDepth = 6;

    GroupConfig group1;
    group1.isleDefaults = baseCfg1;
    group1.intraSubgroupProb = 0.70;
    group1.interSubgroupProb = 0.20;
    group1.subgroups = {
        SubGroupConfig{ 3, std::nullopt }, // SG 1.0 — 3 isles
    };

    IslandManager manager(
        { group0, group1 },
        /*migrationInterval*/ 5,
        /*migrantCount*/ 5,
        /*eliteFraction*/ 0.6
    );
    manager.updateData(X, Y);
    manager.run(
        /*totalGenerations*/ 100,
        /*maxPop*/ 300,
        /*eliteSize*/ 30,
        /*newbornSize*/ 30,
        /*lr*/ 0.05,
        /*cstOptiStep*/ 50,
        /*debug*/ true,
        /*timeoutSeconds*/ 3600
    );

    NodePtr best = manager.bestTree();
    if (best)
        std::cout << "Best: " << printTree(best.get()) << "\n";
}
```

### `IslandManager`

```cpp
IslandManager(
    const std::vector<GroupConfig>& groups,
    unsigned int migrationInterval = 5, // generations between migrations
    unsigned int migrantCount = 5, // individuals exchanged per migration
    double eliteFraction = 0.6 // fraction of migrants taken from elites
);
```

| Method | Description |
|---|---|
| `updateData(X, Y)` | Push dataset to all isles |
| `run(...)` | Run the full island evolution loop |
| `bestTree()` | Best tree found across all isles |
| `bestIsle()` | Isle that holds the best tree |
| `allIsles()` | Flat list of all isles (read-only) |

### `IsleConfig` fields

| Field | Type | Description |
|---|---|---|
| `variables` | `vector<string>` | Variable names |
| `maxDepth` | `unsigned int` | Max tree depth for this isle |
| `penalty` | `double` | Complexity penalty |
| `mutationProb` | `double` | Mutation probability |
| `probs` | `tuple<double,double,double>` | (const, var, binary) probabilities |
| `populationSize` | `size_t` | Population size per isle |
| `k` | `unsigned int` | Tournament size |
| `binaryOps` | `BinaryMap` | Binary operator set |
| `unaryOps` | `UnaryMap` | Unary operator set |

### `GroupConfig` fields

| Field | Type | Description |
|---|---|---|
| `isleDefaults` | `IsleConfig` | Base config inherited by all isles in this group |
| `subgroups` | `vector<SubGroupConfig>` | List of subgroups |
| `intraSubgroupProb` | `double` | Migration probability toward same subgroup |
| `interSubgroupProb` | `double` | Migration probability toward another subgroup in this group |

`interGroupProb` is implicit: `1 - intraSubgroupProb - interSubgroupProb`.

### `SubGroupConfig` fields

| Field | Type | Description |
|---|---|---|
| `numIsles` | `int` | Number of isles in this subgroup |
| `isleOverride` | `optional<IsleConfig>` | If set, overrides group defaults for all isles in this subgroup |

### When to use the island model

| Situation | Recommendation |
|---|---|
| Simple function, depth <= 4 | Single `ModelManager` is sufficient |
| Unknown operator mix needed | Use 2–3 groups with different operator sets |
| Population stagnates early | Add more isles or increase `migrantCount` |
| Need both exploration and exploitation | Use subgroup overrides with different `mutationProb` |

### Tips

- **Migration interval**: 5–10 generations is a good default. Too frequent and all isles converge together; too rare and the islands never benefit from each other.
- **`eliteFraction`**: 0.6 means 60% of migrants are elites, 40% are random. Lower values increase diversity of migrants; higher values accelerate exploitation of good structures.
