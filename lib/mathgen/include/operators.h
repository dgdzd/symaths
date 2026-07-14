#ifndef SYMATHS_OPERATORS_H
#define SYMATHS_OPERATORS_H

#include <string>
#include <unordered_map>
#include <cmath>

using BinaryFunc = double(*)(double, double);
using UnaryFunc = double(*)(double);
using NaryFunc   = double(*)(const std::vector<double>&);

using BinaryMap = std::unordered_map<std::string, BinaryFunc>;
using UnaryMap = std::unordered_map<std::string, UnaryFunc>;
using NaryMap = std::unordered_map<std::string, NaryFunc>;

struct Operators {
    BinaryMap binary;
    UnaryMap unary;
    NaryMap nary;

    Operators() {
        binary = defaultBinary();
        unary = defaultUnary();
        nary = defaultNary();
    }
    Operators(const BinaryMap& b, const UnaryMap& u, const NaryMap& n) {
        if (b.empty()) binary = defaultBinary();
        else  binary = b;

        if (u.empty()) unary = defaultUnary();
        else unary = u;

        if (n.empty()) nary = defaultNary();
        else nary = n;
    }

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

    static double nsum(const std::vector<double>& v)  { double s = 0; for(double x : v) { s +=x; } return s; }

    static NaryMap defaultNary() {
        return {
            { "sum", nsum }
        };
    }
};

#endif