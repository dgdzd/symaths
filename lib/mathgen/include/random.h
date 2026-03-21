#ifndef SYMATHS_RANDOM_H
#define SYMATHS_RANDOM_H

#include <numeric>
#include <random>
#include <vector>
#include <string>
#include <stdexcept>

inline std::mt19937& rng() {
    thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

inline void seedRng(unsigned int seed) {
    rng().seed(seed);
}

inline double randDouble(double lo, double hi) {
    return std::uniform_real_distribution<double>{lo, hi}(rng());
}

inline double randGauss(double mu = 0.0, double sigma = 1.0) {
    return std::normal_distribution<double>{mu, sigma}(rng());
}

inline int randInt(int lo, int hi) {
    return std::uniform_int_distribution<int>{lo, hi}(rng());
}

inline bool randBool(double prob) {
    return std::bernoulli_distribution{prob}(rng());
}

template<typename T>
const T& randChoice(const std::vector<T>& v) {
    if (v.empty()) throw std::runtime_error("randChoice: empty vector");
    return v[randInt(0, static_cast<int>(v.size()) - 1)];
}

template<typename Map>
const std::string& randKey(const Map& m) {
    static thread_local std::vector<std::string> keys;
    keys.clear();
    keys.reserve(m.size());
    for (const auto& [k, _] : m) keys.push_back(k);
    return randChoice(keys);
}

inline std::vector<int> sampleIndices(int size, int n) {
    if (n > size) throw std::runtime_error("sampleIndices: n > size");
    std::vector<int> idx(size);
    std::iota(idx.begin(), idx.end(), 0);
    std::ranges::shuffle(idx, rng());
    idx.resize(n);
    return idx;
}


#endif