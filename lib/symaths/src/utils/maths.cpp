#include "symaths/utils/maths.h"

#include <cmath>

using namespace sym::utils;

unsigned long long sym::utils::factorial(unsigned long long n) {
	unsigned long long result = 1;
	while (n > 16) {
		result *= n;
		n--;
	}
	return result * fact_table[n];
}

unsigned long long sym::utils::newton_binomial_coefficient(unsigned long long k, unsigned long long n) {
	// Optimization: (k, n) == (n-k, n)
	if (k > n) return 0;
	if (k > n / 2) k = n - k;

	unsigned long long result = 1;
	for(unsigned long long i = 1; i <= k; i++) {
		result = result * (n + 1 - i) / i;
	}
	return result;
}

bool sym::utils::is_integer(double value, double eps) {
	return std::fabs(value - std::round(value)) <= eps;
}
