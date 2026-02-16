#ifndef SYMATHS_LIBRARY_HPP
#define SYMATHS_LIBRARY_HPP

#include "symaths/detail/nodes.hpp"
#include "symaths/differentiation.hpp"
#include "symaths/expressions_manip.hpp"

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

	class expression {
	public:
		const detail::node* root;

		expression(double val) : root(make_constant(val)) {}
		expression(const std::string& name) : root(make_symbol(name)) {}
		expression(const char* name) : root(make_symbol(name)) {}
		expression(const detail::node* node) : root(node) {}

		double operator()(const detail::Context& ctx) const { return root->eval(&ctx); }
		double operator()() const { return root->eval(nullptr); }
		[[nodiscard]] std::string string() const { return root->string(nullptr); }

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

		friend expression operator+(const expression& lhs, const expression& rhs) {
			auto add = make_addition({lhs.root, rhs.root});
			return {add};
		}

		friend expression operator-(const expression& lhs, const expression& rhs) {
			auto nrhs = make_negation(rhs.root);
			return lhs + nrhs;
		}

		friend expression operator*(const expression& lhs, const expression& rhs) {
			auto mul = make_multiplication({lhs.root, rhs.root});
			return {mul};
		}

		friend expression operator/(const expression& lhs, const expression& rhs) {
			auto div = make_div(lhs.root, rhs.root);
			return {div};
		}

		friend expression operator-(const expression& e) {
			return make_negation(e.root);
		}

		friend expression pow(const expression& lhs, const expression& rhs);
	};

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

	class variable {
		const detail::node* m_ref;

		friend expression sym::differentiate(const expression& expr, const variable& symbol);

	public:
		variable();
		variable(const std::string& name);
		variable(const char* name);
		variable(const expression& expr);
		variable(const detail::node* root);

		[[nodiscard]] const std::string& name() const;
		const std::string& name(const std::string& new_name);

		operator expression() const;
	};
}

#endif