#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "symaths/detail/nodes.hpp"

namespace sym {
	class symbol;
	class function;

	class expression {
	public:
		const detail::node* root;

		expression(double val);
		expression(const symbol& var);
		expression(const std::string& name);
		expression(const char* name);
		expression(const detail::node* node);

		double operator()(const detail::Context& ctx) const;
		double operator()() const;
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
