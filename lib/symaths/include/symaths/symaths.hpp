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

#ifndef SYMATHS_LIBRARY_HPP
#define SYMATHS_LIBRARY_HPP

#include "symaths/differentiation.hpp"
#include "symaths/expression.hpp"
#include "symaths/expressions_manip.hpp"
#include "symaths/symbol.hpp"
#include "symaths/detail/node_manager.hpp"
#include "symaths/parsing/parser.hpp"

namespace sym {
	struct print_policies_t {
		struct sum_t {
			unsigned int operand_spaces = 0;
		};
		struct product_t {
			unsigned int operand_spaces = 0;
			bool use_stars_for_subexprs = false;
		};
		struct power_t {
			unsigned int operand_spaces_before = 0;
			unsigned int operand_spaces_after = 0;
		};

		sum_t sum;
		product_t product;
		power_t power;
	};

	struct refactoring_rules_t {
		bool keep_ground_functions = true;
		unsigned int max_power_expansion_terms = 6;
	};


	class library {
		print_policies_t m_print_policies;
		node_manager_t m_node_manager;
		refactoring_rules_t m_refactoring_rules;

	public:
		library();
		~library();

		[[nodiscard]] const print_policies_t& print_policies() const;
		[[nodiscard]] print_policies_t& print_policies();
		[[nodiscard]] const node_manager_t& node_manager() const;
		[[nodiscard]] node_manager_t& node_manager();
		[[nodiscard]] const refactoring_rules_t& refactoring_rules() const;
		[[nodiscard]] refactoring_rules_t& refactoring_rules();
	};

	extern library* current_context;

	void make_context_current(library& ctx);
	library* get_current_context();

	const detail::expression_node* make_constant(double val);
	const detail::expression_node* make_symbol(const std::string& name);
	const detail::expression_node* make_negation(const detail::expression_node* node);
	const detail::expression_node* make_addition(const std::vector<const detail::expression_node*>& operands);
	const detail::expression_node* make_multiplication(const std::vector<const detail::expression_node*>& operands);
	const detail::expression_node* make_div(const detail::expression_node* numerator, const detail::expression_node* denominator);
	const detail::expression_node* make_power(const detail::expression_node* base, const detail::expression_node* exponent);
	const detail::expression_node* make_func(uint32_t f_id, const detail::expression_node* arg);
	const detail::expression_node* make_func(uint32_t f_id, const std::vector<const detail::expression_node*>& args);

	expression pow(const expression& lhs, const expression& rhs);
	expression cos(const expression& arg);
	expression sin(const expression& arg);
	expression tan(const expression& arg);
	expression acos(const expression& arg);
	expression asin(const expression& arg);
	expression atan(const expression& arg);
	expression exp(const expression& arg);
	expression ln(const expression& arg);
	expression log10(const expression& arg);
	expression cosh(const expression& arg);
	expression sinh(const expression& arg);
	expression tanh(const expression& arg);
	expression sqrt(const expression& arg);
	expression abs(const expression& arg);
}

#endif