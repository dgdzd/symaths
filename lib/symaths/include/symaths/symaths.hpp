#ifndef SYMATHS_LIBRARY_HPP
#define SYMATHS_LIBRARY_HPP

#include "symaths/differentiation.hpp"
#include "symaths/expression.hpp"
#include "symaths/expressions_manip.hpp"
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


	class library {
		print_policies_t m_print_policies;
		node_manager_t m_node_manager;

	public:
		library();
		~library();

		[[nodiscard]] const print_policies_t& print_policies() const;
		[[nodiscard]] print_policies_t& print_policies();
		[[nodiscard]] const node_manager_t& node_manager() const;
		[[nodiscard]] node_manager_t& node_manager();
	};

	extern library* current_context;

	void make_context_current(library& ctx);
	library* get_current_context();

	const detail::node* make_constant(double val);
	const detail::node* make_symbol(const std::string& name);
	const detail::node* make_negation(const detail::node* node);
	const detail::node* make_addition(const std::vector<const detail::node*>& operands);
	const detail::node* make_multiplication(const std::vector<const detail::node*>& operands);
	const detail::node* make_div(const detail::node* numerator, const detail::node* denominator);
	const detail::node* make_power(const detail::node* base, const detail::node* exponent);
	const detail::node* make_func(uint32_t f_id, const detail::node* arg);
	const detail::node* make_func(uint32_t f_id, const std::vector<const detail::node*>& args);

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

	class symbol {
		const detail::node* m_ref;

		friend expression;
		friend expression sym::differentiate(const expression& expr, const symbol& symbol);

	public:
		symbol();
		symbol(const std::string& name);
		symbol(const char* name);
		symbol(const expression& expr);
		symbol(const detail::node* root);

		[[nodiscard]] const std::string& name() const;
		const std::string& name(const std::string& new_name);
	};

	/**
	 * @brief Class representing a single-argument mathematical function
	 */
	class function {
		const detail::node* m_ref;

		friend expression;

	public:
		function();
	};
}

#endif