#include "symaths/expressions_manip.hpp"

#include "symaths/detail/nodes.h"

/*bool is_simplifyable(sym::detail::NodePtr) {
	// First check if there are numbers (no variables)
	if (auto constant = )
	if (auto sum = std::dynamic_pointer_cast<>())
}

sym::expression sym::detail::simplify_additions(NodePtr parent) {
	if (auto sum = std::dynamic_pointer_cast<objs::addition>(parent)) {
		sum->
	}
}

sym::expression sym::detail::simplify_multiplications(NodePtr parent) {

}



bool sym::is_constant(expression expr) {
	if
}*/

sym::expression sym::reduce(const expression& expr) {
	if (auto op = std::dynamic_pointer_cast<detail::n_operation>(expr.root))
		return sort(op->reduced());

	return sort(expr);
}

sym::expression sym::sort(const expression& expr) {
	if (auto operation = std::dynamic_pointer_cast<detail::n_operation>(expr.root))
		return {operation->sorted()};

	return expr;
}

sym::expression sym::expand(const expression& expr) {
	if (auto mul = std::dynamic_pointer_cast<objs::multiplication>(expr.root)) {
		return {mul->expand()};
	}

	return expr;
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/*size_t sym::hash(const expression& expr) {
	auto h1 = std::hash<int>{}(static_cast<int>(expr.root->kind()));
	auto h2 = std::hash<int>{}(static_cast<int>(expr.root->priority()));
}*/

/*double sym::get_power(const expression& expr) {
	if (auto pow = std::dynamic_pointer_cast<objs::power>(expr.root)) {

	}
}*/

sym::detail::term sym::detail::extract_term(NodePtr node) {
	term t{};
	if (auto mult = std::dynamic_pointer_cast<objs::multiplication>(node)) {
		mult->flatten();
		auto smult = mult->sorted();
		t.coefficient = 1.0;
		for (auto& op : smult->get_operands()) {
			if (op->is_ground()) {
				t.coefficient *= op->eval(nullptr);
			}
			else {
				if (t.symbolic) {
					auto symb = std::static_pointer_cast<objs::multiplication>(t.symbolic);
					symb->add_operand(op);
				}
				else {
					t.symbolic = std::make_shared<objs::multiplication>(op);
				}
			}
		}
	}
	else {
		t.coefficient = 1.0;
		t.symbolic = node;
	}

	return t;
}

/*sym::detail::AdditionNodePtr sym::detail::develop(MultiplicationNodePtr node) {

}*/

