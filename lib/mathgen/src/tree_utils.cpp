#include "tree_utils.h"
#include "fitness.h"
#include "random.h"

#include <sstream>
#include <iomanip>
#include <algorithm>


struct NodeWithParent {
    Node* node;
    Node* parent; //nullptr for root
    bool  isLeft; //for BinaryNosde
};

static void collectWithParent(Node* node, Node* parent, bool isLeft, std::vector<NodeWithParent>& out) {
    out.push_back({node, parent, isLeft});
    if (auto* u = dynamic_cast<UnaryNode*>(node))
        collectWithParent(u->child.get(), node, true, out);
    else if (auto* b = dynamic_cast<BinaryNode*>(node)) {
        collectWithParent(b->left.get(),  node, true,  out);
        collectWithParent(b->right.get(), node, false, out);
    }
}

static void replaceChild(Node* parent, Node* oldChild, NodePtr replacement) {
    if (auto* u = dynamic_cast<UnaryNode*>(parent)) {
        if (u->child.get() == oldChild) u->child = std::move(replacement);
    }
    else if (auto* b = dynamic_cast<BinaryNode*>(parent)) {
        if (b->left.get()  == oldChild) b->left  = std::move(replacement);
        else if (b->right.get() == oldChild) b->right = std::move(replacement);
    }
}

NodePtr randomTree(unsigned int maxDepth, const std::vector<std::string>& variables, std::tuple<double, double, double> probs, const UnaryMap& unaryFuncs, const BinaryMap& binaryFuncs) {
    const auto [constProb, varProb, binaryProb] = probs;

    if (maxDepth == 0)
        return std::make_unique<ConstNode>(randDouble(-10.0, 10.0));

    double r = randDouble(0.0, 1.0);

    if (r < constProb)
        return std::make_unique<ConstNode>(randDouble(-10.0, 10.0));
    if (r < constProb + varProb)
        return std::make_unique<VarNode>(randChoice(variables));
    if (r < constProb + varProb + binaryProb) {
        const std::string& op = randKey(binaryFuncs);
        auto left  = randomTree(maxDepth - 1, variables, probs, unaryFuncs, binaryFuncs);
        auto right = randomTree(maxDepth - 1, variables, probs, unaryFuncs, binaryFuncs);
        return std::make_unique<BinaryNode>(op, binaryFuncs.at(op), std::move(left), std::move(right));
    }
    //unary
    const std::string& name = randKey(unaryFuncs);
    auto child = randomTree(maxDepth - 1, variables, probs, unaryFuncs, binaryFuncs);
    return std::make_unique<UnaryNode>(name, unaryFuncs.at(name), std::move(child));
}

std::string printTree(const Node* node, const std::unordered_map<std::string, std::string>& aliases) {
    if (!node) return "null";

    if (const auto* c = dynamic_cast<const ConstNode*>(node)) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << c->value;
        return ss.str();
    }

    if (const auto* v = dynamic_cast<const VarNode*>(node)) {
        return v->name;
    }

    if (const auto* u = dynamic_cast<const UnaryNode*>(node)) {
        std::string childStr = printTree(u->child.get());

        auto it = aliases.find(u->name);
        if (it != aliases.end()) {
            const std::string& alias = it->second;

            if (alias == "²" || alias == "³")
                return childStr + alias;
            if (alias == "√")
                return "√" + childStr;
            if (alias == "|")
                return "|" + childStr + "|";
            return alias + "(" + childStr + ")";
        }

        return u->name + "(" + childStr + ")";
    }
    if (const auto* b = dynamic_cast<const BinaryNode*>(node)) {
        auto it = aliases.find(b->op);
        if (it != aliases.end()) {
            return "(" + printTree(b->left.get()) + " " + it->second + " " + printTree(b->right.get()) + ")";
        }
        return "(" + printTree(b->left.get()) + " " + b->op + " " + printTree(b->right.get()) + ")";
    }
    return "?";
}

NodePtr mutateSubtree(NodePtr node, unsigned int maxDepth, const std::vector<std::string>& variables, double mutProb, const std::tuple<double, double, double>& probs, const UnaryMap& unaryFuncs,
    const BinaryMap& binaryFuncs) {

    if (randBool(mutProb))
        return randomTree(maxDepth, variables, probs, unaryFuncs, binaryFuncs);

    if (auto* u = dynamic_cast<UnaryNode*>(node.get())) {
        u->child = mutateSubtree(std::move(u->child), maxDepth - 1, variables, mutProb, probs, unaryFuncs, binaryFuncs);
    }
    else if (auto* b = dynamic_cast<BinaryNode*>(node.get())) {
        b->left  = mutateSubtree(std::move(b->left),  maxDepth - 1, variables, mutProb, probs, unaryFuncs, binaryFuncs);
        b->right = mutateSubtree(std::move(b->right), maxDepth - 1, variables, mutProb, probs, unaryFuncs, binaryFuncs);
    }
    return node;
}

void mutateConstants(Node* node, double sigma) {
    if (auto* c = dynamic_cast<ConstNode*>(node)) {
        c->value = clamp(c->value + randGauss(0.0, sigma), -100.0, 100.0);
        return;
    }
    if (auto* u = dynamic_cast<UnaryNode*>(node)) {
        mutateConstants(u->child.get(), sigma);
    }
    else if (auto* b = dynamic_cast<BinaryNode*>(node)) {
        mutateConstants(b->left.get(),  sigma);
        mutateConstants(b->right.get(), sigma);
    }
}

void mutateOperator(Node* node, double prob, const BinaryMap& binaryFuncs, const UnaryMap& unaryFuncs) {
    if (auto* b = dynamic_cast<BinaryNode*>(node)) {
        if (randBool(prob)) {
            const std::string& newOp = randKey(binaryFuncs);
            b->op   = newOp;
            b->func = binaryFuncs.at(newOp);
        }
        mutateOperator(b->left.get(),  prob, binaryFuncs, unaryFuncs);
        mutateOperator(b->right.get(), prob, binaryFuncs, unaryFuncs);
    }
    else if (auto* u = dynamic_cast<UnaryNode*>(node)) {
        if (randBool(prob)) {
            const std::string& newName = randKey(unaryFuncs);
            u->name = newName;
            u->func = unaryFuncs.at(newName);
        }
        mutateOperator(u->child.get(), prob, binaryFuncs, unaryFuncs);
    }
}

NodePtr crossover(const Node* parent1, const Node* parent2) {
    NodePtr child = parent1->clone();

    std::vector<NodeWithParent> nodes1, nodes2Dummy;
    collectWithParent(child.get(),   nullptr, true, nodes1);
    collectWithParent(const_cast<Node*>(parent2), nullptr, true, nodes2Dummy);

    auto all2 = const_cast<Node*>(parent2)->nodes();

    auto& [n1, p1, isLeft1] = nodes1[randInt(0, static_cast<int>(nodes1.size()) - 1)];
    Node* n2 = all2[randInt(0, static_cast<int>(all2.size()) - 1)];

    NodePtr replacement = n2->clone();

    if (p1 == nullptr) {
        return replacement;
    }

    replaceChild(p1, n1, std::move(replacement));
    return child;
}

NodePtr prune(NodePtr node) {
    if (!node) return node;

    if (auto* u = dynamic_cast<UnaryNode*>(node.get())) {
        u->child = prune(std::move(u->child));
        if (isConstantSubtree(u->child.get())) {
            Sample empty{ };
            return std::make_unique<ConstNode>(node->eval(empty));
        }
        return node;
    }

    if (auto* b = dynamic_cast<BinaryNode*>(node.get())) {
        b->left = prune(std::move(b->left));
        b->right = prune(std::move(b->right));

        const bool leftIsConst = b->left->type() == Node::Type::Const;
        const bool rightIsConst = b->right->type() == Node::Type::Const;

        if (isConstantSubtree(b)) {
            Sample empty{ };
            return std::make_unique<ConstNode>(node->eval(empty));
        }

        auto* lc = leftIsConst ? dynamic_cast<ConstNode*>(b->left.get()) : nullptr;
        auto* rc = rightIsConst ? dynamic_cast<ConstNode*>(b->right.get()) : nullptr;

        // (structural equality: same VarNode name)
        if (auto* lv = dynamic_cast<VarNode*>(b->left.get())) {
            if (auto* rv = dynamic_cast<VarNode*>(b->right.get())) {
                if (lv->name == rv->name) {
                    if (b->op == "-") return std::make_unique<ConstNode>(0.0);
                    if (b->op == "/") return std::make_unique<ConstNode>(1.0);
                }
            }
        }

        if (rc) {
            if (b->op == "+" && rc->value == 0.0) return std::move(b->left);
            if (b->op == "-" && rc->value == 0.0) return std::move(b->left);
            if (b->op == "*" && rc->value == 1.0) return std::move(b->left);
            if (b->op == "*" && rc->value == 0.0) return std::make_unique<ConstNode>(0.0);
            if (b->op == "/" && rc->value == 1.0) return std::move(b->left);
            if (b->op == "/" && rc->value == 0.0) return std::make_unique<ConstNode>(0.0);
            if (b->op == "*" && rc->value == -1.0) {
                auto zero = std::make_unique<ConstNode>(0.0);
                return std::make_unique<BinaryNode>("-", b->func, std::move(zero), std::move(b->left));
            }
        }

        if (lc) {
            if (b->op == "+" && lc->value == 0.0) return std::move(b->right);
            if (b->op == "-" && lc->value == 0.0) { }
            if (b->op == "*" && lc->value == 1.0) return std::move(b->right);
            if (b->op == "*" && lc->value == 0.0) return std::make_unique<ConstNode>(0.0);
            if (b->op == "/" && lc->value == 0.0) return std::make_unique<ConstNode>(0.0);
            if (b->op == "*" && lc->value == -1.0) {
                auto zero = std::make_unique<ConstNode>(0.0);
                return std::make_unique<BinaryNode>("-", b->func, std::move(zero), std::move(b->right));
            }
        }

        if (rc && (b->op == "*" || b->op == "+")) {
            if (auto* inner = dynamic_cast<BinaryNode*>(b->left.get())) {
                if (inner->op == b->op) {
                    if (auto* ic = dynamic_cast<ConstNode*>(inner->right.get())) {
                        double merged = (b->op == "*") ? ic->value * rc->value : ic->value + rc->value;
                        auto mergedNode = std::make_unique<ConstNode>(merged);
                        return std::make_unique<BinaryNode>(b->op, b->func, std::move(inner->left), std::move(mergedNode));
                    }
                }
            }
        }

        if (b->op == "/") {
            if (auto* lv = dynamic_cast<VarNode*>(b->left.get())) {
                if (auto* inner = dynamic_cast<BinaryNode*>(b->right.get())) {
                    if (inner->op == "*") {
                        if (auto* rv = dynamic_cast<VarNode*>(inner->left.get())) {
                            if (lv->name == rv->name) {
                                if (auto* ic = dynamic_cast<ConstNode*>(inner->right.get())) {
                                    double val = std::abs(ic->value) > 1e-12 ? 1.0 / ic->value : 0.0;
                                    return std::make_unique<ConstNode>(val);
                                }
                            }
                        }
                    }
                }
            }
        }
        return node;
    }
    return node;
}

void cropSimilarTrees(std::vector<std::unique_ptr<Node>> trees, const Dataset& X) {
    for (size_t i = 0; i < trees.size(); i++) {
        for (size_t j = i + 1; j < trees.size(); j++) {
            double similarity = 0.0;
            for (const Sample& k : X)
                similarity += std::abs(trees[i]->eval(k) - trees[j]->eval(k));

            if (similarity < 1e3) {
                trees.erase(trees.begin() + static_cast<long long>(i));
                i++;
                j = i + 1;
            }
        }
    }
}
