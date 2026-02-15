#include "symaths/differentiation.hpp"

#include "symaths/symaths.hpp"

using namespace sym;

expression sym::differentiate(const expression& expr, const variable& symbol) {
	return reduce(std::visit([&](const auto& x) -> expression {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::constant>) {
			return current_context->node_manager().make_constant(0);
		}

		else if constexpr (std::is_same_v<T, detail::symbol>) {
			if (symbol.m_ref == expr.root) {
				return current_context->node_manager().make_constant(1);
			}
			return current_context->node_manager().make_constant(0);
		}

		else if constexpr (std::is_same_v<T, detail::negation>) {
			return current_context->node_manager().make_negation(differentiate(x.child, symbol).root);
		}

		else if constexpr (std::is_same_v<T, detail::addition>) {
			std::vector<const detail::node*> result;
			// (f + g + h + ...)' = f' + g' + h' + ...
			for (auto& op : x.operands) {
				result.push_back(differentiate(op, symbol).root);
			}
			return current_context->node_manager().make_add(result);
		}

		// Product rule
		else if constexpr (std::is_same_v<T, detail::multiplication>) {
			std::vector<const detail::node*> result;
			// (f * g * h) = f'gh + fg'h + fgh'
			for (auto& f : x.operands) {
				std::vector<const detail::node*> group_product;
				group_product.push_back(differentiate(f, symbol).root);
				for (auto& g : x.operands) {
					if (&f == &g) continue;
					group_product.push_back(g);
				}
				result.push_back(current_context->node_manager().make_mul(group_product));
			}
			return current_context->node_manager().make_add(result);
		}

		return expr.root;
	}, expr.root->p_data));
}
