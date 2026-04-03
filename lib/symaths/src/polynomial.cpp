#include "symaths/polynomial.hpp"

#include "symaths/expressions_manip.hpp"

#include <stdexcept>

using namespace sym;

void polynomial::check_expr_validity(const detail::node* node, const detail::node* variable) const {
	std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::negation>) {
			check_expr_validity(x.child);
		}

		if constexpr (std::is_same_v<T, detail::addition>) {
			for (auto op : x.operands) {
				detail::term t = detail::extract_term(op);
				auto symlist = detail::list_symbols(t.symbolic);
				if (!symlist.empty()) {
					if (std::holds_alternative<detail::symbol>(symlist[0]->p_data)) {
						auto symb_ = std::get<detail::symbol>(symlist[0]->p_data);
						if (symb_.name != symb.name) {
							throw std::invalid_argument("Wrong polynomial variable");
						}
					}
				}

			}
		}
	}, node->p_data);
}


polynomial::polynomial(const expression& root) : expr(reduce(expand(root))) {
	auto symbols_list = detail::list_symbols(expr.root);
	if (symbols_list.size() > 1) {
		throw std::invalid_argument("Expression is an invalid polynomial : too many symbols.");
	}
	if (!symbols_list.empty()) {
		symb = symbols_list[0];
	}
	check_expr_validity(expr.root);
}
