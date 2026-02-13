#include "symaths/symaths.hpp"

#include "symaths/detail/nodes.h"

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
		throw std::runtime_error("sym::make_constant: current context is null");
	}
	return current_context->node_manager().make_negation(node);
}

const sym::detail::node* sym::make_addition(const std::vector<const detail::node*>& operands) {
	if (!current_context) {
		throw std::runtime_error("sym::make_constant: current context is null");
	}
	return current_context->node_manager().make_add(operands);
}

const sym::detail::node* sym::make_multiplication(const std::vector<const detail::node*>& operands) {
	if (!current_context) {
		throw std::runtime_error("sym::make_constant: current context is null");
	}
	return current_context->node_manager().make_mul(operands);
}

const sym::detail::node* sym::make_div(const detail::node* numerator, const detail::node* denominator) {
	if (!current_context) {
		throw std::runtime_error("sym::make_constant: current context is null");
	}
	return current_context->node_manager().make_div(numerator, denominator);
}

const sym::detail::node* sym::make_power(const detail::node* base, const detail::node* exponent) {
	if (!current_context) {
		throw std::runtime_error("sym::make_constant: current context is null");
	}
	return current_context->node_manager().make_pow(base, exponent);
}
