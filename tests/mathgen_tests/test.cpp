#include <../../mathgen/include/model_manager.h>
#include <cmath>
#include <iostream>

int main() {
    //1. Define operators
    BinaryMap binaryFunc = {
        {"+", [](double a, double b){ return a + b; }},
        {"-", [](double a, double b){ return a - b; }},
        {"*", [](double a, double b){ return a * b; }},
        {"/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; }},
    };
    UnaryMap unaryFunc = {
        { "sin", [](double x){ return std::sin(x); }},
        { "cos", [](double x) { return std::cos(x); }},
        { "square", [](double x){ return x * x; }},
        {"exp", [](double x){ return std::abs(x) < 10.0 ? std::exp(x) : 0.0; }},
        { "log", [](double x) { return x <= 0.0 ? 0.0 : std::log(x); }}
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
        Y.push_back(std::log(x_v * x_v + std::sin(x_v + exp(x_v))));
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
        /*lr*/ 0.05,
        /*optimisation steps*/ 20,
        /*debug*/ true,
        /*timeoutSecs*/ 3600,
        [](double fitness) { return fitness < 1e-3; }
    );

    std::cout << manager.getTree(0);

}
