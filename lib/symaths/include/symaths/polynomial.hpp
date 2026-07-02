/*
 *	                            _   _
 *	  ___ _   _ _ __ ___   __ _| |_| |__  ___
 *	 / __| | | | '_ ` _ \ / _` | __| '_ \/ __|   Symbolic maths for C++
 *	 \__ \ |_| | | | | | | (_| | |_| | | \__ \   Version : 0.0.1
 *	 |___/\__, |_| |_| |_|\__,_|\__|_| |_|___/   https://github.com/dgdzd/symaths
 *		  |___/
 *
 * All source code is distributed under the GNU General Public License v2.0.
 *
 */

#ifndef SYM_POLYNOMIAL_HPP
#define SYM_POLYNOMIAL_HPP

#include <map>

#include "symaths/expression.hpp"
#include "symaths/symbol.hpp"

namespace sym {
	class polynomial {
		void validate_expr(const detail::expression_node* node, const detail::expression_node* variable = nullptr);

	public:
		symbol symb;
		std::vector<const detail::expression_node*> coeffs;
		expression expr;

		polynomial(const expression& root);

		unsigned long long get_degree() const;
	};
}

#endif
