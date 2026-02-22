#include "symaths/expression.hpp"

#include "symaths/symaths.hpp"


sym::expression::expression(double val) {
	root = make_constant(val);
}

sym::expression::expression(const symbol& var) {
	root = var.m_ref;
}

sym::expression::expression(const std::string& name) {
	root = make_symbol(name);
}

sym::expression::expression(const char* name) {
	root = make_symbol(name);
}

sym::expression::expression(const detail::node* node) {
	root = node;
}

double sym::expression::operator()(const detail::Context& ctx) const {
	return root->eval(&ctx);
}

double sym::expression::operator()() const {
	return root->eval(nullptr);
}

std::string sym::expression::string() const {
	return root->string(nullptr);
}


sym::expression sym::operator+(const expression& lhs, const expression& rhs) {
	auto add = make_addition({lhs.root, rhs.root});
	return {add};
}

sym::expression sym::operator-(const expression& lhs, const expression& rhs) {
	auto nrhs = make_negation(rhs.root);
	return lhs + nrhs;
}

sym::expression sym::operator*(const expression& lhs, const expression& rhs) {
	auto mul = make_multiplication({lhs.root, rhs.root});
	return {mul};
}

sym::expression sym::operator/(const expression& lhs, const expression& rhs) {
	auto div = make_div(lhs.root, rhs.root);
	return {div};
}

sym::expression sym::operator-(const expression& e) {
	return make_negation(e.root);
}