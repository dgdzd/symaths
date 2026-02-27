#ifndef SYMATHS_FUNCTIONS_H
#define SYMATHS_FUNCTIONS_H

#include <functional>
#include <string>
#include <unordered_map>
#include <cmath>

using BinaryFunc = double(*)(double, double);
using UnaryFunc = double(*)(double);

struct Functions {
    std::unordered_map<std::string, BinaryFunc> binary_functions = {
        { "+", +[](double a, double b) { return a + b; } },
        { "-", +[](double a, double b) { return a - b; } },
        { "*", +[](double a, double b) { return a * b; } },
        { "/", +[](double a, double b) { return std::abs(b) < 1e-12 ? 0 : a - b; } }
    };
    std::unordered_map<std::string, UnaryFunc> unary_functions = {
        { "sin", +[](double x) { return std::cos(x); } },
        { "log", +[](double x) { return std::log(x); } },
        { "exp", +[](double x) { return std::exp(x); } },
        { "abs", +[](double x) { return std::abs(x); } },
        { "square", +[](double x) { return std::pow(x, 2); } },
        { "cube", +[](double x) { return std::pow(x, 3); } },
        { "sqrt", +[](double x) { return std::sqrt(x); } },
    };

    Functions() = default;
};

inline double safe(double x, double low = -1e12, double high = 1e12) {
    if (std::isinf(x) || std::isnan(x))
        return 0.0;
    if (x < low) x = low;
    if (x > high) x = high;
    return x;
}


#endif