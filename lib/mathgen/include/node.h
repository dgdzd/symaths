#ifndef SYMATHS_NODE_H
#define SYMATHS_NODE_H

#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>
#include <unordered_map>

#include "functions.h"

struct ConstNode;
struct VarNode;
struct UnaryNode;
struct BinaryNode;

using Node = std::variant<ConstNode, VarNode, UnaryNode, BinaryNode>;

//magie noire
template<class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;


struct ConstNode {
    double value;

    ConstNode(double v) : value(v) {}

    ConstNode() {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<int> dis(-10, 10);
        value = dis(rng);
    }
};

struct VarNode {
    std::string name;
};

struct UnaryNode {
    std::string name;
    UnaryFunc func;
    std::shared_ptr<Node> child;
};

struct BinaryNode {
    std::string name;
    BinaryFunc func;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
};


inline double get_const_value(const Node& node) {
    double val = std::visit([]<typename T0>(T0&& n) -> double {
        if constexpr (std::is_same_v<std::decay_t<T0>, ConstNode>) return n.value;
        else
            return 0.0;
    }, expr.value);
}

inline double eval(const Node& node, const std::unordered_map<std::string, double>& vars) {
    return std::visit(Overloaded{
        [&](const ConstNode& n) {
            return n.value;
        },
        [&](const VarNode& n) {
            if (!vars.contains(n.name))
                throw std::runtime_error("Variable " + n.name + " does not exist");
            return vars.at(n.name);
        },
        [&](const UnaryNode& n) {
            return safe(n.func(eval(*n.child, vars)));
        },
        [&](const BinaryNode& n) {
            return safe(n.func(
                eval(*n.left, vars),
                eval(*n.right, vars)
            ));
        }
    }, node);
}

inline size_t complexity(const Node& node) {
    return std::visit(Overloaded{
        [](const ConstNode&) -> size_t {
            return 1;
        },
        [](const VarNode&) -> size_t {
            return 1;
        },
        [](const UnaryNode& n) -> size_t {
            return 1 + complexity(*n.child);
        },
        [](const BinaryNode& n) -> size_t {
            return 1 + complexity(*n.left) + complexity(*n.right);
        }
    }, node);
}

inline std::vector<Node> nodes(const Node& node) {
    return std::visit(Overloaded{
        [](const ConstNode& n) {
            std::vector<Node> v;
            v.emplace_back(n);
            return v;
        },
        [](const VarNode& n) {
            std::vector<Node> v;
            v.emplace_back(n);
            return v;
        },
        [](const UnaryNode& n) {
            auto v = nodes(*n.child);
            v.emplace_back(n);
            return v;
        },
        [](const BinaryNode& n) {
            auto v = nodes(*n.left);
            auto r = nodes(*n.right);
            v.insert(v.end(), r.begin(), r.end());
            v.emplace_back(n);
            return v;
        }
    }, node);
}

#endif