#ifndef SYMATHS_CMA_ES_H
#define SYMATHS_CMA_ES_H

#include "matrix.h"
#include "params.h"

#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <stdexcept>



class CMAES {
public:

    CMAES(int n, const CMAESConfig& cfg = {}) : params(n, cfg), n(n), sigma(cfg.sigma0), gen(std::random_device{}()), normal(0.0, 1.0)
        , iter(0), eigen_iter(0), stall_count(0), best_fit(std::numeric_limits<double>::infinity()) {

        mean = Vector(n, 0.0);
        p_sigma = Vector(n, 0.0);
        p_c = Vector(n, 0.0);
        C = make_identity(n);
        values.resize(n, 1.0);
        vectors = make_identity(n);
        BD = make_identity(n); // B*D = I(0) (C = I)
        best_x = Vector(n, 0.0);

        population.resize(params.lambda, Vector(n));
        ys.resize(params.lambda, Vector(n));
    }

    void set_mean(const Vector& x0) {
        if (static_cast<int>(x0.size()) != n)
            throw std::invalid_argument("set_mean: wrong dimensions");
        mean = x0;
        best_x = x0;
    }

    // returns ref to population
    const std::vector<Vector>& ask() {
        for (int k = 0; k < params.lambda; k++) {
            Vector z(n);
            for (int i = 0; i < n; i++)
                z[i] = normal(gen);

            ys[k] = mv_mul(BD, z);

            for (int i = 0; i < n; i++)
                population[k][i] = mean[i] + sigma * ys[k][i];
        }
        return population;
    }

    //get fintnessses & update
    void tell(const std::vector<double>& fitnesses) {
        if ((int)fitnesses.size() != params.lambda)
            throw std::invalid_argument("tell: wrong fitnesses size");

        auto& w = params.weights;
        int mu = params.mu;

        std::vector<int> idx(params.lambda);
        std::iota(idx.begin(), idx.end(), 0);
        std::ranges::sort(idx, [&](int a, int b){ return fitnesses[a] < fitnesses[b]; });

        if (fitnesses[idx[0]] < best_fit) {
            best_fit = fitnesses[idx[0]];
            best_x = population[idx[0]];
        }

        Vector old_mean = mean;
        mean = Vector(n, 0.0);
        for (int i = 0; i < mu; i++)
            for (int d = 0; d < n; d++)
                mean[d] += w[i] * population[idx[i]][d];

        Vector y_mean(n, 0.0);
        for (int i = 0; i < mu; i++)
            for (int d = 0; d < n; d++)
                y_mean[d] += w[i] * ys[idx[i]][d];

        Vector v(n, 0.0);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                v[i] += vectors[j][i] * y_mean[j];

        for (int i = 0; i < n; i++)
            v[i] /= std::sqrt(values[i]);

        Vector invcSqrt_y = mv_mul(vectors, v);

        double c1  = params.cs;
        double damp = std::sqrt(c1 * (2.0 - c1) * params.mu_eff);
        for (int d = 0; d < n; d++)
            p_sigma[d] = (1.0 - params.cs) * p_sigma[d] + damp * invcSqrt_y[d];

        double ps_norm = norm(p_sigma);
        sigma *= std::exp((params.cs / params.ds) * (ps_norm / params.chiN - 1.0));

        sigma = std::clamp(sigma, 1e-12, 1e6);

        double h_thresh = (1.4 + 2.0 / (n + 1.0)) * params.chiN;
        int h_sigma = (ps_norm / std::sqrt(1.0 - std::pow(1.0 - params.cs, 2.0 * (iter + 1)))) < h_thresh ? 1 : 0;

        double damp_c = std::sqrt(params.cc * (2.0 - params.cc) * params.mu_eff);
        for (int d = 0; d < n; d++)
            p_c[d] = (1.0 - params.cc) * p_c[d] + h_sigma * damp_c * y_mean[d];

        double coeff_keep = 1.0 - params.c1 - params.cmu;
        double delta_h = (1.0 - h_sigma) * params.cc * (2.0 - params.cc);

        Matrix rank_1 = outer(p_c, p_c);

        Matrix rank_mu = make_matrix(n, 0.0);
        for (int i = 0; i < mu; i++) {
            Matrix yyt = outer(ys[idx[i]], ys[idx[i]]);
            for (int r = 0; r < n; r++)
                for (int c = 0; c < n; c++)
                    rank_mu[r][c] += w[i] * yyt[r][c];
        }

        for (int r = 0; r < n; r++)
            for (int c = 0; c < n; c++)
                C[r][c] = coeff_keep * C[r][c] + params.c1  * (rank_1[r][c] + delta_h * C[r][c]) + params.cmu * rank_mu[r][c];

        eigen_iter++;
        if (eigen_iter >= params.eigen_every) {
            eigen_iter = 0;
            decomp(C, values, vectors);
            BD = make_BD(vectors, values);
        }

        check_convergence(fitnesses, idx, old_mean);

        iter++;

        if (params.debug)
            printf("[%4d]  sigma=%.3e  best=%.6e\n", iter, sigma, best_fit);
    }

    //takes a fitness function f and a vector of init coeffs
    //returns optimized coeffs
    template<typename Fitness>
    Vector optimize(Fitness f, const Vector& x0) {
        set_mean(x0);
        converged_ = false;
        stall_count = 0;
        iter = 0;

        std::vector<double> fitnesses(params.lambda);

        while (!converged_ && iter < params.max_iter) {
            const auto& pop = ask();
            for (int k = 0; k < params.lambda; k++)
                fitnesses[k] = f(pop[k]);
            tell(fitnesses);
        }
        return best_x;
    }


    const Vector& best() const {
        return best_x;
    }
    double best_fitness() const {
        return best_fit;
    }
    int generation() const {
        return iter;
    }
    double step_size() const {
        return sigma;
    }
    bool converged() const {
        return converged_;
    }

private:
    CMAESParams params;
    int n;

    Vector mean;
    double sigma;
    Matrix C;
    Vector p_sigma;
    Vector p_c;

    Vector values;
    Matrix vectors;
    Matrix BD;

    std::vector<Vector> population;
    std::vector<Vector> ys;

    Vector best_x;
    double best_fit;

    std::mt19937_64 gen;
    std::normal_distribution<> normal;

    int iter;
    int eigen_iter;
    int stall_count;
    bool converged_ = false;

    void check_convergence(const std::vector<double>& fitnesses, const std::vector<int>& idx, const Vector& old_mean) {
        double dx = 0.0;
        for (int d = 0; d < n; d++)
            dx = std::max(dx, std::abs(mean[d] - old_mean[d]));

        double df = std::abs(fitnesses[idx[0]] - fitnesses[idx[params.lambda - 1]]);

        if (dx < params.tol_x && df < params.tol_fun)
            stall_count++;
        else
            stall_count = 0;

        if (stall_count >= 10)
            converged_ = true;

        if (sigma < 1e-12)
            converged_ = true;
    }
};



#endif