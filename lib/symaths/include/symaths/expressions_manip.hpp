#ifndef EXPRESSIONS_MANIP_HPP
#define EXPRESSIONS_MANIP_HPP

#include "symaths.hpp"

namespace sym {
	namespace detail {
		expression simplify_additions(NodePtr parent);
		expression simplify_multiplications(NodePtr parent);
	}

	/**
	 * @brief This function takes an expression and removes redundant operations between known values.
	 *
	 * Simplifying arranges in the descending order of powers of the unknown variables until constants.
	 * For example :
	 * - reduce(3 + x + 10) = x + 13
	 * - reduce(3 * 4 * x) = 12x
	 * - reduce(3 + 4 * 3 + x) = x + 15
	 * - reduce(3 * (2 + 7) * x) = 27x
	 * - reduce(2 * x^2 + 3 * x + 6 * x) = 2x^2 + 9x
	 *
	 * @param expr The expression to reduce
	 * @return The simplified expression
	 */
	expression reduce(expression expr);

	bool is_constant(expression expr);

	namespace detail {
		struct term {
			double coefficient;
			NodePtr symbolic;
		};

		term extract_term(NodePtr node);

		AdditionNodePtr develop(MultiplicationNodePtr node);
	}
}

#endif
