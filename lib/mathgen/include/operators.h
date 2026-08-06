#ifndef SYMATHS_OPERATORS_H
#define SYMATHS_OPERATORS_H

#include <string>
#include <unordered_map>
#include <cmath>
#include <vector>

using UnaryFunc = double(*)(double);
using BinaryFunc = double(*)(double, double);
using TrinaryFunc = double(*)(double, double, double);
using NaryFunc = double(*)(const std::vector<double>&);

using UnaryMap = std::unordered_map<std::string, UnaryFunc>;
using BinaryMap = std::unordered_map<std::string, BinaryFunc>;
using TrinaryMap = std::unordered_map<std::string, TrinaryFunc>;
using NaryMap = std::unordered_map<std::string, NaryFunc>;

struct Operators {
    UnaryMap unary;
    BinaryMap binary;
    TrinaryMap trinary;
    NaryMap nary;

    Operators() {
        unary = defaultUnary();
        binary = defaultBinary();
        trinary = defaultTrinary();
        nary = defaultNary();
    }
    Operators(const UnaryMap& u, const BinaryMap& b, const TrinaryMap& t, const NaryMap& n) {
        if (u.empty()) unary = defaultUnary();
        else unary = u;

        if (b.empty()) binary = defaultBinary();
        else binary = b;

        if (t.empty()) trinary = defaultTrinary();
        else trinary = t;

        if (n.empty()) nary = defaultNary();
        else nary = n;
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

    static double condition(double a, double b, double c) { return std::abs(a) < 1e-6 ? b : c; }
    static double fma_f(double a, double b, double c) { return std::fma(a, b, c); }

    static TrinaryMap defaultTrinary() {
        return {
            { "condition", condition },
            { "fma", fma_f }
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