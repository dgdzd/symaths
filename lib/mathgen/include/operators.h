#ifndef SYMATHS_OPERATORS_H
#define SYMATHS_OPERATORS_H

#include <string>
#include <unordered_map>
#include <cmath>

using BinaryFunc = double(*)(double, double);
using UnaryFunc = double(*)(double);

using BinaryMap = std::unordered_map<std::string, BinaryFunc>;
using UnaryMap = std::unordered_map<std::string, UnaryFunc>;

struct Operators {
    BinaryMap binary;
    UnaryMap unary;

    Operators() {
        binary = defaultBinary();
        unary = defaultUnary();
    }

    Operators(BinaryMap b, UnaryMap u) : binary(std::move(b)), unary(std::move(u)) {}

    static double add(double a, double b) { return a + b; }
    static double sub(double a, double b) { return a - b; }
    static double mul(double a, double b) { return a * b; }
    static double div(double a, double b) { return std::abs(b) > 1e-12 ? a / b : 0.0; }

    static BinaryMap defaultBinary() {
        return {
            { "+", add },
            { "-", sub },
            { "*", mul },
            { "/", div },
        };
    }

    static double sin_f(double x) { return std::sin(x); }
    static double cos_f(double x) { return std::cos(x); }
    static double tanh_f(double x) { return std::tanh(x); }
    static double log_f(double x) { return std::log(std::abs(x) + 1e-12); }
    static double exp_f(double x) {
        return std::abs(x) < 10.0 ? std::exp(x) : 0.0;
    }
    static double abs_f(double x) { return std::abs(x); }
    static double square(double x) { return x * x; }
    static double cube(double x) { return x * x * x; }
    static double sqrt_f(double x) { return std::sqrt(std::abs(x)); }

    static UnaryMap defaultUnary() {
        return {
            { "sin", sin_f },
            { "cos", cos_f },
            { "tanh", tanh_f },
            { "log", log_f },
            { "exp", exp_f },
            { "abs", abs_f },
            { "square", square },
            { "cube", cube },
            { "sqrt", sqrt_f },
        };
    }

    void mergeUnary(const UnaryMap& extra) {
        for (const auto& [k, v] : extra)
            unary[k] = v;
    }
};

#endif