#ifndef SYMATHS_MUTATIONS_H
#define SYMATHS_MUTATIONS_H

#include "node.h"
#include "functions.h"

#include <random>

template<typename type>
float uniform_random(type a = 0, type b = 1) {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_real_distribution<type> dist(a, b); // [a, b)
    return dist(rng);
}

template<typename type>
inline double normal_dis(type a = 0, type b = 1) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::normal_distribution<> dist(a, b);
    return dist(rng);
}

template<typename key_type, typename value_type>
std::vector<std::pair<key_type, value_type>> map_to_vec(const std::unordered_map<key_type, value_type>& map) {
    std::vector<std::pair<key_type, value_type>> vec;
    for (const auto& pair : map) {
        vec.push_back(pair);
    }
    return vec;
}


inline Node random_tree(size_t max_depth, std::vector<std::string> variables, std::tuple<float, float, float> probs, const Functions& func) {

}

inline Node mutate_subtree(Node node,
                           size_t max_depth,
                           const std::vector<std::string>& variables,
                           float mut_prob,
                           const std::tuple<float,float,float>& probs,
                           const Functions& func)
{
    if (uniform_random<float>() < mut_prob) {
        return random_tree(max_depth, variables, probs, func);
    }

    std::visit([&](auto& n){
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, UnaryNode>) {
            *n.child = mutate_subtree(std::move(*n.child), max_depth-1, variables, mut_prob, probs, func);
        } else if constexpr (std::is_same_v<T, BinaryNode>) {
            *n.left  = mutate_subtree(std::move(*n.left),  max_depth-1, variables, mut_prob, probs, func);
            *n.right = mutate_subtree(std::move(*n.right), max_depth-1, variables, mut_prob, probs, func);
        }
    }, node.value);

    return node;
}

inline void mutate_constants(Node& node, float sigma = 0.2f) {
    std::visit([&](auto& n){
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, ConstNode>) {
            n.value = safe(n.value + normal_dis<double>(-sigma, sigma));
        } else if constexpr (std::is_same_v<T, UnaryNode>) {
            mutate_constants(*n.child, sigma);
        } else if constexpr (std::is_same_v<T, BinaryNode>) {
            mutate_constants(*n.left, sigma);
            mutate_constants(*n.right, sigma);
        }
    }, node.value);
}

inline void mutate_binary_func(Node& node, float prob, const Functions& func) {
    std::visit([&](auto& n){
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, BinaryNode>) {
            if (uniform_random<float>() < prob) {
                auto vec = map_to_vec(func.binary_functions);
                auto idx = static_cast<size_t>(uniform_random<int>(0, vec.size() - 1));
                n.name = vec[idx].first;
                n.func = vec[idx].second;
            }
            mutate_binary_func(*n.left, prob, func);
            mutate_binary_func(*n.right, prob, func);
        } else if constexpr (std::is_same_v<T, UnaryNode>) {
            mutate_binary_func(*n.child, prob, func);
        }
    }, node.value);
}


inline void all_nodes_with_parent(Node& node, Node* parent, std::vector<std::pair<Node*, Node*>>& out) {
    out.emplace_back(&node, parent);

    std::visit([&](auto& n){
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, UnaryNode>) {
            all_nodes_with_parent(*n.child, &node, out);
        } else if constexpr (std::is_same_v<T, BinaryNode>) {
            all_nodes_with_parent(*n.left,  &node, out);
            all_nodes_with_parent(*n.right, &node, out);
        }
    }, node.value);
}

inline std::vector<std::pair<Node*, Node*>> all_nodes_with_parent(Node& root) {
    std::vector<std::pair<Node*, Node*>> result;
    all_nodes_with_parent(root, nullptr, result);
    return result;
}

#endif