#ifndef SYMATHS_PARAMS_H
#define SYMATHS_PARAMS_H


#include <cmath>
#include <vector>
#include <algorithm>
#include <stdexcept>


struct CMAESConfig {
    int lambda = -1; //-1 = auto (pop size)
    int mu = -1; //-1 = auto (number of selected parents)
    double sigma0 = 1.0; // initial step size
    unsigned int max_iter = 1000;
    double tol_fun = 1e-6; // stops if delta_f < tol
    double tol_x = 1e-6; // stops if delta_mean < tol
    bool debug = false;
};

struct CMAESParams {
    int n;
    int lambda;
    int mu;

    std::vector<double> weights;
    double mu_eff;

    unsigned int max_iter;

    double sigma0;
    double cs; // lr (du pas)
    double ds; // damping
    double chiN; // predicted value

    double cc; // lr (de la cumulation de pas)
    double c1; // lr (du rang 1)
    double cmu; // lr (du rang micro)

    double tol_fun;
    double tol_x;

    bool debug;

    int eigen_every; // calculate C every N gens

    CMAESParams(int n_, const CMAESConfig& cfg) : n(n_) {
        if (n <= 0)
            throw std::invalid_argument("CMAESParams: n doit être > 0");

        lambda = (cfg.lambda > 0) ? cfg.lambda : 4 + static_cast<int>(std::floor(3.0 * std::log(n)));
        mu = (cfg.mu > 0 && cfg.mu < lambda) ? cfg.mu : lambda / 2;

        if (lambda < 2)
            throw std::invalid_argument("CMAESParams: lambda doit être >= 2");

        weights.resize(mu);
        double sum_w = 0.0;
        for (int i = 0; i < mu; i++) {
            weights[i] = std::log(mu + 0.5) - std::log(i + 1.0);
            sum_w += weights[i];
        }
        for (auto& w : weights) w /= sum_w;  // normalization -> sum = 1

        //mu_eff
        double sum_w2 = 0.0;
        for (auto& w : weights) sum_w2 += w * w;
        mu_eff = 1.0 / sum_w2;

        max_iter = cfg.max_iter;

        //sigma
        sigma0 = cfg.sigma0;
        cs = (mu_eff + 2.0) / (n + mu_eff + 5.0);
        ds = 1.0 + cs + 2.0 * std::max(0.0, std::sqrt((mu_eff - 1.0) / (n + 1.0)) - 1.0);
        chiN = std::sqrt(static_cast<double>(n)) * (1.0 - 1.0 / (4.0 * n) + 1.0 / (21.0 * n * n));

        //covariance
        cc  = (4.0 + mu_eff / n) / (n + 4.0 + 2.0 * mu_eff / n);
        c1  = 2.0 / (std::pow(n + 1.3, 2.0) + mu_eff);
        cmu = std::min(1.0 - c1, 2.0 * (mu_eff - 2.0 + 1.0 / mu_eff) / (std::pow(n + 2.0, 2.0) + mu_eff));

        tol_fun = cfg.tol_fun;
        tol_x = cfg.tol_x;

        debug = cfg.debug;

        //eigen_freq
        eigen_every = std::max(1, static_cast<int>(std::floor(1.0 / (c1 + cmu) / n / 10.0)));
    }

    void log() const {
        printf("n=%d  lambda=%d  mu=%d  mu_eff=%.3f\n", n, lambda, mu, mu_eff);
        printf("cs=%.4f  ds=%.4f  chiN=%.4f\n", cs, ds, chiN);
        printf("cc=%.4f  c1=%.5f  cmu=%.5f\n", cc, c1, cmu);
        printf("eigen_every=%d\n", eigen_every);
    }
};

#endif