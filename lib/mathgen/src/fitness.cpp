#include "fitness.h"
#include "cma-es/cma_es.h"

#include <stdexcept>

double fitness(Node* tree, const Dataset& X, const std::vector<double>& Y, double penalty, size_t gen,size_t maxGen) {
    if (X.size() != Y.size())
        throw std::runtime_error("fitness: X and Y size mismatch");

    double err = 0.0;
    const std::size_t n = Y.size();

    for (std::size_t i = 0; i < n; i++) {
        double pred = tree->eval(X[i]);
        err += std::abs(pred - Y[i]);
    }
    err /= static_cast<double>(n);

    double genRatio = (maxGen != 0) ? static_cast<double>(gen) / static_cast<double>(maxGen) : 0.0;
    double f_fitness = err + penalty * genRatio * tree->complexity();
    return (std::isnan(f_fitness) || std::isinf(f_fitness)) ? 1e20 : f_fitness;
}

void optimizeConstants(Node* tree, const Dataset& X, const std::vector<double>& Y, const CMAESConfig& cfg, size_t k) {
    std::vector<ConstNode*> constNodes;
    for (Node* n : tree->nodes())
        if (auto* c = dynamic_cast<ConstNode*>(n))
            constNodes.push_back(c); //false error
    if (constNodes.empty()) return;

    const size_t n = constNodes.size();

    std::vector<double> x(n);
    for (size_t i = 0; i < n; i++)
        x[i] = constNodes[i]->value;

    auto applyAndEval = [&](const std::vector<double>& vals) {
        for (size_t i = 0; i < n; i++)
            constNodes[i]->value = vals[i];
        return fitness(tree, X, Y, 0.0, 0, 1);
    };

    if (n == 1) {
        // golden section search in [x[0] - 5, x[0] + 5]
        double lo = x[0] - 5.0, hi = x[0] + 5.0;
        const double phi = (std::sqrt(5.0) - 1.0) / 2.0;
        for (int i = 0; i < 50; i++) {
            double m1 = hi - phi * (hi - lo);
            double m2 = lo + phi * (hi - lo);
            if (applyAndEval({m1}) < applyAndEval({m2})) hi = m2;
            else lo = m1;
        }
        x[0] = (lo + hi) / 2.0;
    }
    else if (n < k) {
        optiBFGS(x, applyAndEval, n, cfg.tol_fun, cfg.max_iter);
    }
    else {
        CMAES optimizer(n, cfg);
        Vector init(n);
        for (size_t i = 0; i < n; i++)
            init[i] = constNodes[i]->value;
        optimizer.set_mean(init);

        while (!optimizer.converged() && optimizer.generation() < cfg.max_iter) {
            const auto& candidates = optimizer.ask();

            std::vector<double> scores(candidates.size());
            for (size_t k = 0; k < candidates.size(); k++) {
                for (size_t i = 0; i < n; i++)
                    constNodes[i]->value = candidates[k][i];

                scores[k] = fitness(tree, X, Y, 0.0, 0, 1);
            }

            optimizer.tell(scores);
        }

        Vector best_coeffs = optimizer.best();
        for (size_t i = 0; i < best_coeffs.size(); i++)
            constNodes[i]->value = best_coeffs[i];
    }
}


//BFGS with finite difference gradient
void optiBFGS(std::vector<double>& x, const std::function<double(const std::vector<double>& vals)>& applyAndEval, size_t n, const double tol, const size_t max_iter) {
    const double h = 1e-6;

    // Hessian inverse approx (identity)
    std::vector<std::vector<double>> H(n, std::vector<double>(n, 0.0));
    for (size_t i = 0; i < n; i++) H[i][i] = 1.0;

    auto computeGrad = [&](const std::vector<double>& vals, std::vector<double>& g) {
        double f0 = applyAndEval(vals);
        std::vector<double> perturbed = vals;
        for (size_t i = 0; i < n; i++) {
            perturbed[i] += h;
            g[i] = (applyAndEval(perturbed) - f0) / h;
            perturbed[i] = vals[i];
        }
        return f0;
    };

    std::vector<double> grad(n), prev_grad(n), prev_x(n);
    double f = computeGrad(x, grad);

    for (size_t iter = 0; iter < max_iter; iter++) {
        // Convergence check
        double gnorm = 0.0;
        for (double g : grad) gnorm += g * g;
        if (std::sqrt(gnorm) < tol) break;

        // Search direction d = -H * grad
        std::vector<double> d(n, 0.0);
        for (size_t i = 0; i < n; i++)
            for (size_t j = 0; j < n; j++)
                d[i] -= H[i][j] * grad[j];

        double dot_gd = 0.0;
        for (size_t i = 0; i < n; i++) dot_gd += grad[i] * d[i];
        if (dot_gd >= 0.0) {
            for (size_t i = 0; i < n; i++)
                for (size_t j = 0; j < n; j++)
                    H[i][j] = (i == j) ? 1.0 : 0.0;
            for (size_t i = 0; i < n; i++) d[i] = -grad[i];
            dot_gd = 0.0;
            for (size_t i = 0; i < n; i++) dot_gd += grad[i] * d[i];
        }

        //Backtracking line search (Armijo)
        double alpha = 1.0;
        for (int ls = 0; ls < 30; ls++) {
            std::vector<double> x_new(n);
            for (size_t i = 0; i < n; i++) x_new[i] = x[i] + alpha * d[i];
            if (applyAndEval(x_new) <= f + 1e-4 * alpha * dot_gd) break;
            alpha *= 0.5;
        }

        prev_x   = x;
        prev_grad = grad;
        for (size_t i = 0; i < n; i++) x[i] += alpha * d[i];
        f = computeGrad(x, grad);

        // s = x_new - x_old,  y = grad_new - grad_old
        std::vector<double> s(n), y(n);
        double sy = 0.0;
        for (size_t i = 0; i < n; i++) {
            s[i] = x[i] - prev_x[i];
            y[i] = grad[i] - prev_grad[i];
            sy += s[i] * y[i];
        }

        if (sy < 1e-10) continue; //skip update if not positive definite

        // H = (I - rho * s * y ^ T) * H * (I - rho * y * s ^ T) + rho * s * s ^ T
        double rho = 1.0 / sy;

        //Hy = H * y
        std::vector<double> Hy(n, 0.0);
        for (size_t i = 0; i < n; i++)
            for (size_t j = 0; j < n; j++)
                Hy[i] += H[i][j] * y[j];

        //yHy = y^T * H * y
        double yHy = 0.0;
        for (size_t i = 0; i < n; i++)
            yHy += y[i] * Hy[i];

        //rank-2 update: H += (rho^2 * yHy + rho) * s*s^T - rho * (Hy*s^T + s*Hy^T)
        for (size_t i = 0; i < n; i++)
            for (size_t j = 0; j < n; j++)
                H[i][j] += (rho * rho * yHy + rho) * s[i] * s[j] - rho * (Hy[i] * s[j] + s[i] * Hy[j]);
    }
}