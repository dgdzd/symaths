#ifndef EXPRESSIONS_MANIP_HPP
#define EXPRESSIONS_MANIP_HPP

namespace sym {
	class expression;
	namespace detail {
		class node;
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
	expression reduce(const expression& expr);
	expression sort(const expression& expr);
	expression expand(const expression& expr);
	//size_t hash(const expression& expr);

	namespace detail {
		struct term {
			double coefficient;
			const node* symbolic;
		};

		term extract_term(const node* node);

		const node* develop(const node* node);
	}
}

#endif
