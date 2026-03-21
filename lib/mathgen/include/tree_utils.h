#ifndef SYMATHS_TREE_UTILS_H
#define SYMATHS_TREE_UTILS_H

#include "node.h"
#include "operators.h"
#include "random.h"

#include <string>
#include <vector>
#include <tuple>


// probs = (const_prob, var_prob, binary_prob);
NodePtr randomTree(int maxDepth, const std::vector<std::string>& variables, std::tuple<double, double, double> probs, const UnaryMap& unaryFuncs, const BinaryMap& binaryFuncs);

std::string printTree(const Node* node);

NodePtr mutateSubtree(NodePtr node, int maxDepth, const std::vector<std::string>& variables, double mutProb, std::tuple<double, double, double> probs, const UnaryMap& unaryFuncs, const BinaryMap& binaryFuncs);

void mutateConstants(Node* node, double sigma = 0.2);

void mutateOperator(Node* node, double prob, const BinaryMap& binaryFuncs, const UnaryMap& unaryFuncs);

NodePtr crossover(const Node* parent1, const Node* parent2);

NodePtr prune(NodePtr node);


#endif