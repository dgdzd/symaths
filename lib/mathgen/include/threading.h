#ifndef SYMATHS_THREADING_H
#define SYMATHS_THREADING_H

#include <thread>
#include <vector>
#include <functional>

inline void parallelFor(size_t n, std::function<void(size_t, size_t)> fn) {
    const size_t nThreads = std::min(n, (size_t)std::thread::hardware_concurrency());
    if (nThreads <= 1) { fn(0, n); return; }

    std::vector<std::thread> threads;
    threads.reserve(nThreads);
    const size_t chunkSize = n / nThreads;

    for (size_t t = 0; t < nThreads; t++) {
        size_t from = t * chunkSize;
        size_t to   = (t == nThreads - 1) ? n : from + chunkSize;
        threads.emplace_back(fn, from, to);
    }
    for (auto& t : threads) t.join();
}

#endif