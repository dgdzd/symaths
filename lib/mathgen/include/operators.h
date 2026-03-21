#ifndef SYMATHS_OPERATORS_H
#define SYMATHS_OPERATORS_H

#include <functional>
#include <string>
#include <unordered_map>
#include <cmath>

using BinaryFunc = std::function<double(double, double)>;
using UnaryFunc = std::function<double(double)>;
using BinaryMap = std::unordered_map<std::string, BinaryFunc>;
using UnaryMap = std::unordered_map<std::string, UnaryFunc>;

struct Operators {
    BinaryMap binary;
    UnaryMap  unary;

    Operators() {
        binary = defaultBinary();
        unary  = defaultUnary();
    }

    Operators(BinaryMap b, UnaryMap u) : binary(std::move(b)), unary(std::move(u)) {}

    static BinaryMap defaultBinary() {
        return {
            {"+", [](double a, double b){ return a + b; }},
            {"-", [](double a, double b){ return a - b; }},
            {"*", [](double a, double b){ return a * b; }},
            {"/", [](double a, double b){ return std::abs(b) > 1e-12 ? a / b : 0.0; }},
        };
    }

    static UnaryMap defaultUnary() {
        return {
            {"sin",    [](double x){ return std::sin(x); }},
            {"cos",    [](double x){ return std::cos(x); }},
            {"tanh",   [](double x){ return std::tanh(x); }},
            {"log",    [](double x){ return std::log(std::abs(x) + 1e-12); }},
            {"exp",    [](double x){ return std::abs(x) < 10.0 ? std::exp(x) : 0.0; }},
            {"abs",    [](double x){ return std::abs(x); }},
            {"square", [](double x){ return x * x; }},
            {"cube",   [](double x){ return x * x * x; }},
            {"sqrt",   [](double x){ return std::sqrt(std::abs(x)); }},
        };
    }

    void mergeUnary(const UnaryMap& extra) {
        for (const auto& [k, v] : extra)
            unary[k] = v;
    }
};


#endif