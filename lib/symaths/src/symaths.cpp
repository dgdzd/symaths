#include "symaths/symaths.hpp"

#include "symaths/base_functions.hpp"
#include "symaths/detail/nodes.hpp"

#include <stdexcept>

sym::library* sym::current_context = nullptr;

void sym::make_context_current(library& ctx) {
	current_context = &ctx;
}

sym::library* sym::get_current_context() {
	return current_context;
}

sym::library::library() {
	if (!current_context) {
		current_context = this;
	}
	detail::init_builtin_functions();
}

sym::library::~library() {
	if (current_context == this) {
		current_context = nullptr;
	}
}

sym::print_policies_t& sym::library::print_policies() {
	return m_print_policies;
}

const sym::print_policies_t& sym::library::print_policies() const {
	return m_print_policies;
}

sym::node_manager_t& sym::library::node_manager() {
	return m_node_manager;
}

const sym::node_manager_t& sym::library::node_manager() const {
	return m_node_manager;
}

const sym::detail::node* sym::make_constant(double val) {
	if (!current_context) {
		throw std::runtime_error("sym::make_constant: current context is null");
	}
	return current_context->node_manager().make_constant(val);
}

const sym::detail::node* sym::make_symbol(const std::string& name) {
	if (!current_context) {
		throw std::runtime_error("sym::make_symbol: current context is null");
	}
	return current_context->node_manager().make_symbol(name);
}

const sym::detail::node* sym::make_negation(const detail::node* node) {
	if (!current_context) {
		throw std::runtime_error("sym::make_negation: current context is null");
	}
	return current_context->node_manager().make_negation(node);
}

const sym::detail::node* sym::make_addition(const std::vector<const detail::node*>& operands) {
	if (!current_context) {
		throw std::runtime_error("sym::make_addition: current context is null");
	}
	return current_context->node_manager().make_add(operands);
}

const sym::detail::node* sym::make_multiplication(const std::vector<const detail::node*>& operands) {
	if (!current_context) {
		throw std::runtime_error("sym::make_multiplication: current context is null");
	}
	return current_context->node_manager().make_mul(operands);
}

const sym::detail::node* sym::make_div(const detail::node* numerator, const detail::node* denominator) {
	if (!current_context) {
		throw std::runtime_error("sym::make_div: current context is null");
	}
	return current_context->node_manager().make_div(numerator, denominator);
}

const sym::detail::node* sym::make_power(const detail::node* base, const detail::node* exponent) {
	if (!current_context) {
		throw std::runtime_error("sym::make_power: current context is null");
	}
	return current_context->node_manager().make_pow(base, exponent);
}

const sym::detail::node* sym::make_func(uint32_t f_id, const detail::node* arg) {
	if (!current_context) {
		throw std::runtime_error("sym::make_func: current context is null");
	}
	return current_context->node_manager().make_func(f_id, {arg});
}

const sym::detail::node* sym::make_func(uint32_t f_id, const std::vector<const detail::node*>& args) {
	if (!current_context) {
		throw std::runtime_error("sym::make_func: current context is null");
	}
	return current_context->node_manager().make_func(f_id, args);
}


sym::expression sym::pow(const expression& lhs, const expression& rhs) {
	return make_power(lhs.root, rhs.root);
}

sym::expression sym::cos(const expression& arg) {
	return make_func(funcs::cos, arg.root);
}

sym::expression sym::sin(const expression& arg) {
	return make_func(funcs::sin, arg.root);
}

sym::expression sym::tan(const expression& arg) {
	return make_func(funcs::tan, arg.root);
}

sym::expression sym::acos(const expression& arg) {
	return make_func(funcs::acos, arg.root);
}

sym::expression sym::asin(const expression& arg) {
	return make_func(funcs::asin, arg.root);
}

sym::expression sym::atan(const expression& arg) {
	return make_func(funcs::atan, arg.root);
}

sym::expression sym::exp(const expression& arg) {
	return make_func(funcs::exp, arg.root);
}

sym::expression sym::ln(const expression& arg) {
	return make_func(funcs::ln, arg.root);
}

sym::expression sym::log10(const expression& arg) {
	return make_func(funcs::log10, arg.root);
}

sym::expression sym::cosh(const expression& arg) {
	return make_func(funcs::cosh, arg.root);
}

sym::expression sym::sinh(const expression& arg) {
	return make_func(funcs::sinh, arg.root);
}

sym::expression sym::tanh(const expression& arg) {
	return make_func(funcs::tanh, arg.root);
}

sym::expression sym::sqrt(const expression& arg) {
	return make_func(funcs::sqrt, arg.root);
}

sym::expression sym::abs(const expression& arg) {
	return make_func(funcs::abs, arg.root);
}


sym::variable::variable() {
	m_ref = make_symbol("x");
}

sym::variable::variable(const std::string& name) {
	m_ref = make_symbol(name);
}

sym::variable::variable(const char* name) {
	m_ref = make_symbol(name);
}

sym::variable::variable(const expression& expr) {
	if (!std::holds_alternative<detail::symbol>(expr.root->p_data)) {
		throw std::invalid_argument("sym::symbol: expression is not a symbol");
	}
	m_ref = expr.root;
}

sym::variable::variable(const detail::node* root) {
	if (!std::holds_alternative<detail::symbol>(root->p_data)) {
		throw std::invalid_argument("sym::symbol: expression is not a symbol");
	}
	m_ref = root;
}

const std::string& sym::variable::name() const {
	return std::get<detail::symbol>(m_ref->p_data).name;
}

const std::string& sym::variable::name(const std::string& new_name) {
	m_ref = make_symbol(new_name);
	return std::get<detail::symbol>(m_ref->p_data).name;
}

sym::variable::operator sym::expression() const {
	return {m_ref};
}
