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

#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "symaths/detail/nodes.hpp"

namespace sym {
	class symbol;
	class polynomial;
	class function;

	class expression {
	public:
		const detail::node* root;

		expression(double val);
		expression(const symbol& var);
		expression(const polynomial& var);
		expression(const std::string& name);
		expression(const char* name);
		expression(const detail::node* node);

		number operator()(const detail::Context& ctx) const;
		number operator()() const;
		[[nodiscard]] std::string string() const;

		/**
		 * @brief Checks if the expression is composed of constants only
		 *
		 * For example :
		 * - is_ground(3 + ln(4)) == true
		 * - is_ground(3 + 4x) == false
		 *
		 * @return Whether the expression is ground
		 */
		[[nodiscard]] bool is_ground() const { return root->is_ground(); }

		[[nodiscard]] bool operator==(const expression& other) const { return root == other.root; }
	};

	expression operator+(const expression& lhs, const expression& rhs);
	expression operator-(const expression& lhs, const expression& rhs);
	expression operator*(const expression& lhs, const expression& rhs);
	expression operator/(const expression& lhs, const expression& rhs);
	expression operator-(const expression& e);
}

#endif
