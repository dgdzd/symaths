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
