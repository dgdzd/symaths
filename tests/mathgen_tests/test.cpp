#include <../../mathgen/include/model_manager.h>
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

    //2. Build dataset  →  y = x * sin(x) + sin(x^2)
    Dataset X;
    std::vector<double> Y;
    for (int i = -1000; i < 1000; i++) {
        double xv = i / 20.0;
        X.push_back({{"x", xv}});
        Y.push_back(xv * std::sin(xv) + std::sin(xv * xv));
    }

    //3. Configure and run
    ModelManager manager(
        {"x"}, //variable names
        1000, //population size
        7, //max tree depth
        1e-5, //complexity penalty
        0.4, //mutation probability
        {0.1, 0.3, 0.3}, //(const_prob, var_prob, binary_prob)
        7
    );
    manager.initPopulation(binaryFunc, unaryFunc);
    manager.updateData(X, Y);
    manager.fit(
        /*generations*/ 20,
        /*maxPop*/ 1000,
        /*eliteSize*/ 20,
        /*newbornSize*/ 120,
                        0.05,
                        50,
                        true,
        /*timeoutSecs*/ 3600

    );
}