#ifndef SYMATHS_TREE_UTILS_H
#define SYMATHS_TREE_UTILS_H

#include "node.h"
#include "operators.h"
#include "random.h"
#include "fitness.h"

#include <string>
#include <vector>
#include <tuple>

// probs = (const_prob, var_prob, binary_prob, trinary_prob, nary_prob);
using Probs = std::tuple<double, double, double, double, double>;


NodePtr randomTree(unsigned int maxDepth, const std::vector<std::string>& variables, Probs probs, const UnaryMap& unaryFuncs,
    const BinaryMap& binaryFuncs, const TrinaryMap& trinaryFuncs, const NaryMap& naryFuncs);

std::string printTree(const Node* node, const std::unordered_map<std::string,
    std::string>& aliases = { { "square", "^2" }, { "cube", "^3" },{ "abs", "|" } }, unsigned int constPrecision = 3);

NodePtr mutateSubtree(NodePtr node, unsigned int maxDepth, const std::vector<std::string>& variables, double mutProb, Probs probs, const UnaryMap& unaryFuncs,
    const BinaryMap& binaryFuncs, const TrinaryMap& trinaryFuncs, const NaryMap& naryFuncs);

void mutateConstants(Node* node, double sigma = 0.2);

void mutateOperator(Node* node, double prob, const UnaryMap& unaryFuncs, const BinaryMap& binaryFuncs, const TrinaryMap& trinaryFuncs, const NaryMap& naryFuncs);

NodePtr crossover(const Node* parent1, const Node* parent2);

NodePtr prune(NodePtr node);

void cropSimilarTrees(const Dataset& X);

NodePtr strToNode(const std::string& str, const std::vector<std::string>& variables, const UnaryMap& unaryFuncs,
    const BinaryMap& binaryFuncs, const TrinaryMap& trinaryFuncs, const NaryMap& naryFuncs);


#endif