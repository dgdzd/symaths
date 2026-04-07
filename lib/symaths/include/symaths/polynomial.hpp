#ifndef POLYNOMIAL_HPP
#define POLYNOMIAL_HPP

#include <map>

#include "symaths/expression.hpp"
#include "symaths/symbol.hpp"

namespace sym {
	class polynomial {
		void validate_expr(const detail::node* node, const detail::node* variable = nullptr);

	public:
		symbol symb;
		std::vector<const detail::node*> coeffs;
		expression expr;

		polynomial(const expression& root);

		unsigned long long get_degree() const;
	};
}

#endif
