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
fitness = MAE(tree, data) + penalty × (gen / maxGen) × complexity(tree)
```

The complexity penalty increases progressively over generations.


---

## Usage


### Quick start

```cpp
#include "model_manager.h"
#include <cmath>

int main() {
    // 1. Define operators
    BinaryMap binaryFunc = {
        {"+", [](double a, double b){ return a + b; }},
        {"-", [](double a, double b){ return a - b; }},
        {"*", [](double a, double b){ return a * b; }},
        {"/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; }},
    };
    UnaryMap unaryFunc = {
        {"sin",    [](double x){ return std::sin(x); }},
        {"square", [](double x){ return x * x; }},
    };

    // 2. Build dataset  →  y = x * sin(x) + sin(x²)
    Dataset X;
    std::vector<double> Y;
    for (int i = -400; i < 400; ++i) {
        double xv = i / 10.0;
        X.push_back({{"x", xv}});
        Y.push_back(xv * std::sin(xv) + std::sin(xv * xv));
    }

    // 3. Configure and run
    ModelManager manager(
        {"x"},   // variable names
        400,     // population size
        5,       // max tree depth
        1e-5,    // complexity penalty
        0.4,     // mutation probability
        {0.1, 0.3, 0.3}  // (const_prob, var_prob, binary_prob)
    );
    manager.initPopulation(binaryFunc, unaryFunc);
    manager.updateData(X, Y);
    manager.fit(
        /*generations=*/  10,
        /*maxPop=*/       400,
        /*eliteSize=*/    40,
        /*timeoutSecs=*/  3600
    );
}
```

### `ModelManager`

```cpp
ModelManager(
    std::vector<std::string> variables,       // e.g. {"x", "y"}
    int    populationSize = 100,
    int    maxDepth       = 5,
    double penalty        = 0.01,
    double mutationProb   = 0.3,
    std::tuple<double,double,double> probs    // (const, var, binary) — unary gets 1-sum
         = {0.25, 0.25, 0.25}
);
```

| Method | Description |
|---|---|
| `initPopulation(binary, unary, extraUnary)` | Build the initial random population |
| `loadPopulation(population, binary, unary, extraUnary)`| Load any population |
| `getPopulation(sortFitness)`| Return the current population (and sort it if wanted) |
| `updateData(X, Y)` | Set or replace the training dataset |
| `fit(generations, maxPop, eliteSize, timeout, earlyStop)` | Run the evolution loop |
| `residuals(tree)` | Raw residuals `y - ŷ` for a given tree |
| `normalizedResiduals(tree)` | Z-score normalized residuals + scale factor |

### `fit()` parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `generations` | `size_t` | `10` | Number of evolutionary generations |
| `maxPop` | `size_t` | `100` | Maximum population size (≥ 50) |
| `eliteSize` | `size_t` | `10` | Number of top individuals carried over unchanged |
| `debug`| `bool` | `false`| Shows useful informations in the cmd |
| `timeoutSeconds` | `unsigned int` | `60` | Hard wall-clock time limit |
| `earlyStopCondition` | `std::function<bool(double)>` | `nullptr` | Callback receiving best fitness — return `true` to stop early |

### Probability tuple `probs = (const_prob, var_prob, binary_prob)`

Controls the shape of randomly generated trees. The unary probability is implicit: `1 - sum(probs)`. The three values must sum to ≤ 1.0.

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
    {"relu",    [](double x){ return x > 0.0 ? x : 0.0; }},
};
manager.initPopulation(binaryFunc, unaryFunc, extra);
```

---

### Multi-variable regression

```cpp
// Dataset for  z = x² + sin(y)
Dataset X;
std::vector<double> Y;
for (double xv = -3.0; xv <= 3.0; xv += 0.5)
    for (double yv = -3.0; yv <= 3.0; yv += 0.5) {
        X.push_back({{"x", xv}, {"y", yv}});
        Y.push_back(xv * xv + std::sin(yv));
    }

ModelManager manager({"x", "y"}, /*...*/ );
```
