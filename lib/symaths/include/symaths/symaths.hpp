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
#include "symaths/program.hpp"
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
		context_table_t m_context_table;
		std::ostream* m_out;

	public:
		library();
		~library();

		[[nodiscard]] print_policies_t& print_policies();
		[[nodiscard]] const print_policies_t& print_policies() const;
		[[nodiscard]] node_manager_t& node_manager();
		[[nodiscard]] const node_manager_t& node_manager() const;
		[[nodiscard]] refactoring_rules_t& refactoring_rules();
		[[nodiscard]] const refactoring_rules_t& refactoring_rules() const;
		[[nodiscard]] context_table_t& context_table();
		[[nodiscard]] const context_table_t& context_table() const;
		[[nodiscard]] std::ostream*& out_stream();
		[[nodiscard]] std::ostream* out_stream() const;
	};

	extern library* current_context;

	void make_context_current(library& ctx);
	library* get_current_context();

	const detail::mathexpr_node* make_constant(double val);
	const detail::mathexpr_node* make_symbol(const std::string& name);
	const detail::mathexpr_node* make_negation(const detail::mathexpr_node* node);
	const detail::mathexpr_node* make_addition(const std::vector<const detail::mathexpr_node*>& operands);
	const detail::mathexpr_node* make_multiplication(const std::vector<const detail::mathexpr_node*>& operands);
	const detail::mathexpr_node* make_div(const detail::mathexpr_node* numerator, const detail::mathexpr_node* denominator);
	const detail::mathexpr_node* make_power(const detail::mathexpr_node* base, const detail::mathexpr_node* exponent);
	const detail::mathexpr_node* make_func(uint32_t f_id, const detail::mathexpr_node* arg);
	const detail::mathexpr_node* make_func(uint32_t f_id, const std::vector<const detail::mathexpr_node*>& args);

	const detail::statement_node* make_sequence(const std::vector<const detail::statement_node*>& statements);
	const detail::statement_node* make_assignment(const std::string& name, const detail::mathexpr_node* value);
	const detail::statement_node* make_assignment(const std::string& name, const detail::predicate_node* value);
	const detail::statement_node* make_assignment(const std::string& name, const detail::set_node* value);
	const detail::statement_node* make_expression_statement(const detail::mathexpr_node* expr);
	const detail::statement_node* make_expression_statement(const detail::predicate_node* expr);
	const detail::statement_node* make_expression_statement(const detail::set_node* expr);
	const detail::statement_node* make_conditional(const detail::statement_node* condition, const detail::statement_node* then_branch, const detail::statement_node* else_branch);
	/*const detail::statement_node* make_while_loop(const detail::predicate_node* condition, const detail::statement_node* body);
	const detail::statement_node* make_for_loop(const detail::mathexpr_node* variable, const detail::set_node* iterable, const detail::statement_node* body);*/
	const detail::statement_node* make_function_definition(const std::string& name, const std::vector<std::string>& params, const detail::statement_node* body);

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