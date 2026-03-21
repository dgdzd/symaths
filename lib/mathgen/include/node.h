#ifndef SYMATHS_NODE_H
#define SYMATHS_NODE_H

#include "operators.h"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

using Sample = std::unordered_map<std::string, double>;


inline double clamp(double v, double lo = -1e12, double hi = 1e12) {
    if (std::isnan(v) || std::isinf(v)) return 0.0;
    return std::clamp(v, lo, hi);
}

struct Node {
    enum class Type { Const, Var, Unary, Binary };

    virtual ~Node() = default;
    [[nodiscard]] virtual double eval(const Sample& x) const = 0;
    [[nodiscard]] virtual std::unique_ptr<Node> clone() const = 0;
    [[nodiscard]] virtual int complexity() const { return 1; }
    [[nodiscard]] virtual Type type() const = 0;

    virtual void collectNodes(std::vector<Node*>& out) {
        out.push_back(this); //false error
    }
    std::vector<Node*> nodes() {
        std::vector<Node*> out;
        collectNodes(out);
        return out;
    }
};


using NodePtr = std::unique_ptr<Node>;

struct ConstNode final : Node {
    double value;
    explicit ConstNode(double v = 0.0) : value(v) {}
    [[nodiscard]] double eval(const Sample&) const override { return value; }
    [[nodiscard]] NodePtr clone() const override { return std::make_unique<ConstNode>(value); }
    [[nodiscard]] Type type() const override { return Type::Const; }
};

struct VarNode final : Node {
    std::string name;
    explicit VarNode(std::string n) : name(std::move(n)) {}
    [[nodiscard]] double eval(const Sample& x) const override {
        auto it = x.find(name);
        if (it == x.end()) throw std::runtime_error("Unknown variable: " + name);
        return it->second;
    }
    [[nodiscard]] NodePtr clone() const override { return std::make_unique<VarNode>(name); }
    [[nodiscard]] Type type() const override { return Type::Var; }
};

struct UnaryNode final : Node {
    std::string name;
    UnaryFunc func;
    NodePtr child;

    UnaryNode(std::string n, UnaryFunc f, NodePtr c) : name(std::move(n)), func(std::move(f)), child(std::move(c)) {}

    [[nodiscard]] double eval(const Sample& x) const override {
        return clamp(func(child->eval(x)));
    }
    [[nodiscard]] NodePtr clone() const override {
        return std::make_unique<UnaryNode>(name, func, child->clone());
    }
    [[nodiscard]] int complexity() const override {
        return 1 + child->complexity();
    }
    [[nodiscard]] Type type() const override {
        return Type::Unary;
    }
    void collectNodes(std::vector<Node*>& out) override {
        out.push_back(this);
        child->collectNodes(out);
    }
};

struct BinaryNode final : Node {
    std::string op;
    BinaryFunc func;
    NodePtr left;
    NodePtr right;

    BinaryNode(std::string o, BinaryFunc f, NodePtr l, NodePtr r) : op(std::move(o)), func(std::move(f)), left(std::move(l)), right(std::move(r)) {}

    [[nodiscard]] double eval(const Sample& x) const override {
        return clamp(func(left->eval(x), right->eval(x)));
    }
    [[nodiscard]] NodePtr clone() const override {
        return std::make_unique<BinaryNode>(op, func, left->clone(), right->clone());
    }
    [[nodiscard]] int complexity() const override {
        return 1 + left->complexity() + right->complexity();
    }
    [[nodiscard]] Type type() const override {
        return Type::Binary;
    }
    void collectNodes(std::vector<Node*>& out) override {
        out.push_back(this);
        left->collectNodes(out);
        right->collectNodes(out);
    }
};

inline bool isConstantSubtree(const Node* node) {
    if (dynamic_cast<const ConstNode*>(node)) return true;
    if (const auto* u = dynamic_cast<const UnaryNode*>(node))
        return isConstantSubtree(u->child.get());
    if (const auto* b = dynamic_cast<const BinaryNode*>(node))
        return isConstantSubtree(b->left.get()) && isConstantSubtree(b->right.get());
    return false; //var node
}


inline double constantFraction(Node* node) {
    auto all = node->nodes();
    if (all.empty()) return 0.0;
    int consts = 0;
    for (auto* n : all)
        if (n->type() == Node::Type::Const) ++consts;
    return static_cast<double>(consts) / static_cast<double>(all.size());
}

inline bool isMostlyConstants(Node* node, double threshold = 0.9) {
    return constantFraction(node) >= threshold;
}


#endif