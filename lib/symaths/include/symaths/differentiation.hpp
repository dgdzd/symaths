#ifndef DIFFERENTIATION_HPP
#define DIFFERENTIATION_HPP

namespace sym {
	class expression;
	class symbol;

	/**
	 *	@brief Computes the derivative of an expression with respect to a variable.
	 *
	 * @param expr The expression to differentiate
	 * @param symbol The symbol to differentiate the expression with respect to
	 * @return An expression representing the derivative of the input expression
	 */
	expression differentiate(const expression& expr, const symbol& symbol);
}

#endif
