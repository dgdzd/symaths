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

std::string printTree(const Node* node, const std::unordered_map<std::string, std::string>& aliases, unsigned int constPrecision) {
    if (!node) return "null";

    if (const auto* c = dynamic_cast<const ConstNode*>(node)) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(constPrecision) << c->value;
        return ss.str();
    }

    if (const auto* v = dynamic_cast<const VarNode*>(node)) {
        return v->name;
    }

    if (const auto* u = dynamic_cast<const UnaryNode*>(node)) {
        std::string childStr = printTree(u->child.get());
        if (childStr.empty()) return "";
        if (childStr.starts_with("(") && childStr.ends_with(")"))
            childStr = childStr.substr(1, childStr.length() - 2);

        auto it = aliases.find(u->name);
        if (it != aliases.end()) {
            const std::string& alias = it->second;

            if (alias == "^2" || alias == "^3")
                return childStr + alias;
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

std::vector<std::string> tokenize(const std::string& str) {
    std::vector<std::string> tokens;

    std::string s;
    s.reserve(str.size());
    for (char c : str) if (c != ' ') s += c;

    const std::string punctuation = "(),";

    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];

        if (std::isdigit(c) || c == '.') {
            size_t start = i;
            while (i < s.size() && (std::isdigit(s[i]) || s[i] == '.')) ++i;
            tokens.emplace_back(s.substr(start, i - start));
        }

        else if (std::isalpha(c) || c == '_') {
            size_t start = i;
            while (i < s.size() && (std::isalnum(s[i]) || s[i] == '_')) ++i;
            tokens.emplace_back(s.substr(start, i - start));
        }

        else if (punctuation.find(c) != std::string::npos) {
            tokens.emplace_back(1, c);
            ++i;
        }

        else {
            size_t start = i;
            while (i < s.size() && !std::isdigit(s[i]) && !std::isalpha(s[i]) && s[i] != '_' && punctuation.find(s[i]) == std::string::npos && s[i] != ' ')
                ++i;
            tokens.emplace_back(s.substr(start, i - start));
        }
    }
    return tokens;
}

struct OpInfo {
    int prec;
    bool rightAssoc;
};

//precedence
inline OpInfo opInfo(const std::string& op, int default_prec = 20) {
    static const std::unordered_map<std::string, OpInfo> table = {
        { "+", { 10, false } },
        { "-", { 10, false } },
        { "*", { 20, false } },
        { "/", { 20, false } },
        { "%", { 20, false } },
        { "^", { 30, true } },
        { "**", { 30, true } },
    };
    auto it = table.find(op);
    return it != table.end() ? it->second : OpInfo{ default_prec, false };
}

struct Parser {
    const std::vector<std::string>& tokens;
    const std::vector<std::string>& variables;
    const UnaryMap&  unaryFuncs;
    const BinaryMap& binaryFuncs;
    size_t pos = 0;

    [[nodiscard]] bool atEnd() const {
        return pos >= tokens.size();
    }
    [[nodiscard]] const std::string& peek() const {
        if (atEnd()) throw std::runtime_error("Unexpected end of expression");
        return tokens[pos];
    }
    std::string consume() {
        return tokens[pos++];
    }
    [[nodiscard]] bool check(const std::string& s) const {
        return !atEnd() && tokens[pos] == s;
    }
    void expect(const std::string& s) {
        if (atEnd() || tokens[pos] != s)
            throw std::runtime_error("Expected '" + s + "' but got '" + (atEnd() ? "<end>" : peek()) + "'");
        pos++;
    }

    // Returns true if the current token is a known binary operator
    [[nodiscard]] bool isBinaryOp(const std::string& tok) const {
        return binaryFuncs.contains(tok);
    }

    // ── expression entry point (precedence-climbing) ──────────────────────────
    //
    //  parse_expr(minPrec):
    //    lhs = parse_unary()
    //    while next token is a binary op with prec >= minPrec:
    //        op = consume()
    //        nextMinPrec = op.prec + (op.rightAssoc ? 0 : 1)
    //        rhs = parse_expr(nextMinPrec)
    //        lhs = BinaryNode(op, lhs, rhs)
    //    return lhs

    NodePtr parse_expr(int minPrec = 0) {
        NodePtr lhs = parse_unary();

        while (!atEnd()) {
            const std::string& tok = tokens[pos];
            if (!isBinaryOp(tok)) break;

            OpInfo info = opInfo(tok);
            if (info.prec < minPrec) break;

            std::string op = consume();
            int nextMin = info.prec + (info.rightAssoc ? 0 : 1);
            NodePtr rhs = parse_expr(nextMin);

            auto it = binaryFuncs.find(op);
            if (it == binaryFuncs.end())
                throw std::runtime_error("Unknown binary operator: " + op);

            lhs = std::make_unique<BinaryNode>(op, it->second, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    NodePtr parse_unary() {
        if (check("-")) {
            pos++;
            NodePtr operand = parse_unary();
            if (auto* c = dynamic_cast<ConstNode*>(operand.get()))
                return std::make_unique<ConstNode>(-c->value);
            auto it = unaryFuncs.find("neg");
            if (it != unaryFuncs.end())
                return std::make_unique<UnaryNode>("neg", it->second, std::move(operand));

            auto mIt = binaryFuncs.find("*");
            if (mIt == binaryFuncs.end())
                throw std::runtime_error("No unary negation and no '*' operator available");
            return std::make_unique<BinaryNode>("*", mIt->second, std::make_unique<ConstNode>(-1.0), std::move(operand));
        }
        if (check("+")) {
            pos++;
            return parse_unary();
        }

        if (check("(")) {
            pos++;
            NodePtr inner = parse_expr(0);
            expect(")");
            return inner;
        }

        const std::string& tok = peek();

        if (std::isdigit(tok[0]) || tok[0] == '.') {
            pos++;
            return std::make_unique<ConstNode>(std::stod(tok));
        }

        //function or variable
        if (std::isalpha(tok[0]) || tok[0] == '_') {
            std::string name = consume();

            //unary
            if (check("(")) {
                pos++;
                NodePtr arg = parse_expr(0);
                expect(")");

                auto uit = unaryFuncs.find(name);
                if (uit != unaryFuncs.end())
                    return std::make_unique<UnaryNode>(name, uit->second, std::move(arg));

                throw std::runtime_error("Unknown function: " + name);
            }

            if (std::find(variables.begin(), variables.end(), name) != variables.end())
                return std::make_unique<VarNode>(name);

            throw std::runtime_error("Unknown identifier: " + name);
        }

        throw std::runtime_error("Unexpected token: " + tok);
    }
};


NodePtr strToNode(const std::string& str, const std::vector<std::string>& variables, const UnaryMap&  unaryFuncs, const BinaryMap& binaryFuncs) {
    auto tokens = tokenize(str);
    if (tokens.empty()) throw std::runtime_error("Empty expression");

    Parser parser{ tokens, variables, unaryFuncs, binaryFuncs };
    NodePtr result = parser.parse_expr(0);

    if (!parser.atEnd())
        throw std::runtime_error("Unexpected token after expression: " + parser.peek());

    return result;
}