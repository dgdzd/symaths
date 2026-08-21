#include "symaths/expression.hpp"

#include "symaths/program.hpp"
#include "symaths/symaths.hpp"

sym::expression::expression(double val) {
	root = make_constant(val);
}

sym::expression::expression(const object& out) {
	root = out.cast<const detail::mathexpr_node*>();
}

sym::expression::expression(const symbol& var) {
	root = var.ref;
}

sym::expression::expression(const std::string& name) {
	root = make_symbol(name);
}

sym::expression::expression(const char* name) {
	root = make_symbol(name);
}

sym::expression::expression(const detail::mathexpr_node* node) {
	root = node;
}

sym::number sym::expression::operator()(context_table_t& ctx) const {
	if (!root) return numbers::nan{};
	return root->eval(&ctx);
}

sym::number sym::expression::operator()() const {
	if (!root) return numbers::nan{};
	return root->eval(nullptr);
}

std::string sym::expression::string() const {
	if (!root) return "";
	return root->string();
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