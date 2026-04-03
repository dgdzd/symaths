#ifndef POLYNOMIAL_HPP
#define POLYNOMIAL_HPP

#include <map>

#include "symaths/expression.hpp"
#include "symaths/symbol.hpp"

namespace sym {
	class polynomial {
		void check_expr_validity(const detail::node* node, const detail::node* variable = nullptr) const;

	public:
		symbol symb;
		std::vector<double> coeffs;
		expression expr;

		polynomial(const expression& root);

		int get_degree() const;
	};
}

#endif
