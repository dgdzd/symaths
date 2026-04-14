#include "symaths/polynomial.hpp"

#include "symaths/symaths.hpp"
#include "symaths/expressions_manip.hpp"

#include <stdexcept>
#include <symaths/utils/maths.hpp>

using namespace sym;

void polynomial::validate_expr(const detail::node* node, const detail::node* variable) {
	std::visit([&](const auto& x) -> void {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::negation>) {
			validate_expr(x.child, variable);
		}

		if constexpr (std::is_same_v<T, detail::addition>) {
			for (auto op : x.operands) {
				detail::expr_term t = detail::extract_term_advanced(op);
				if (!t.symbolic) {
					if (coeffs.empty()) {
						coeffs.push_back(t.coefficient);
					}
					else {
						coeffs[0] = reduce(current_context->node_manager().make_add({coeffs[0], t.coefficient})).root;
					}
					continue;
				}

				auto symlist = detail::list_symbols(t.symbolic);
				if (symlist[0] != variable) {
					throw std::invalid_argument("Wrong polynomial variable.");
				}

				std::visit([&](const auto& x1) -> void {
					using T = std::decay_t<decltype(x1)>;

					if constexpr (std::is_same_v<T, detail::symbol>) {
						if (coeffs.size() <= 1) {
							auto* zero = current_context->node_manager().make_constant(0);
							coeffs.resize(2, zero);
						}
						coeffs[1] = reduce(current_context->node_manager().make_add({coeffs[1], t.coefficient})).root;
					}

					if constexpr (std::is_same_v<T, detail::power>) {
						if (!x1.exponent->is_ground()) {
							throw std::logic_error("Exponent must be ground");
						}
						if (!utils::is_integer(x1.exponent->eval(nullptr).template get<double>())) {
							throw std::invalid_argument("Exponent is not an integer.");
						}

						auto n = x1.exponent->eval(nullptr).template get<unsigned long long>();
						if (n >= coeffs.size()) {
							auto* zero = current_context->node_manager().make_constant(0);
							coeffs.resize(n + 1, zero);
						}
						coeffs[n] = reduce(current_context->node_manager().make_add({coeffs[n], t.coefficient})).root;
					}
				}, t.symbolic->p_data);
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
	validate_expr(expr.root, symb.ref);
}

unsigned long long polynomial::get_degree() const {
	return coeffs.size() - 1;
}
