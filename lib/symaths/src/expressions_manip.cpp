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
	return std::static_pointer_cast<objs::addition>(expr.root)->reduced();
}

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
	else if (auto var = std::dynamic_pointer_cast<objs::variable>(node)) {
		t.coefficient = 1.0;
		t.symbolic = var;
	}

	return t;
}

/*sym::detail::AdditionNodePtr sym::detail::develop(MultiplicationNodePtr node) {

}*/

