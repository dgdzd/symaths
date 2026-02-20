#include "symaths/differentiation.hpp"

#include "symaths/symaths.hpp"
#include "symaths/base_functions.hpp"

using namespace sym;

expression sym::differentiate(const expression& expr, const symbol& symbol) {
	return reduce(std::visit([&](const auto& x) -> expression {
		using T = std::decay_t<decltype(x)>;

		auto& nm = current_context->node_manager();
		if constexpr (std::is_same_v<T, detail::constant>) {
			return nm.make_constant(0);
		}

		else if constexpr (std::is_same_v<T, detail::symbol>) {
			if (symbol.m_ref == expr.root) {
				return nm.make_constant(1);
			}
			return nm.make_constant(0);
		}

		else if constexpr (std::is_same_v<T, detail::negation>) {
			return nm.make_negation(differentiate(x.child, symbol).root);
		}

		else if constexpr (std::is_same_v<T, detail::addition>) {
			std::vector<const detail::node*> result;
			// (f + g + h + ...)' = f' + g' + h' + ...
			for (auto& op : x.operands) {
				result.push_back(differentiate(op, symbol).root);
			}
			return nm.make_add(result);
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
				result.push_back(nm.make_mul(group_product));
			}
			return nm.make_add(result);
		}

		// Power rule : (x^y)' = yx^(y-1) OR logarithm
		else if constexpr (std::is_same_v<T, detail::power>) {
			if (x.base->depends_on(symbol.m_ref)) {
				// (f(x)^(g(x)))' = (f(x)^(g(x)))(g'(x) ln(f(x)) + f'(x)/f(x))
				if (x.exponent->depends_on(symbol.m_ref)) {
					auto df = sym::differentiate(x.base, symbol).root; // f'(x)
					auto dg = sym::differentiate(x.exponent, symbol).root; // g'(x)

					return nm.make_mul({
						expr.root,
						nm.make_add({
							nm.make_mul({
								dg,
								nm.make_func(funcs::builtin_fn_id::ln, {x.base})
							}),
							nm.make_div(
								df,
								x.base
							)
						})
					});
				}
				// Kind : f(x)^a : (f^a)' = af'f^(a-1)
				auto df = sym::differentiate(x.base, symbol).root;
				return nm.make_mul({
					x.exponent,
					df,
					nm.make_pow(x.base, nm.make_add({x.exponent, nm.make_constant(-1)}))
				});
			}

			// Kind : a^(f(x)) : (a^(f(x)))' = (a^(f(x))) f'(x) ln(a)
			if (x.exponent->depends_on(symbol.m_ref)) {
				auto df = sym::differentiate(x.exponent, symbol).root;
				return nm.make_mul({
					expr.root,
					df,
					nm.make_func(funcs::builtin_fn_id::ln, {x.base})
				});
			}
			return nm.make_constant(0);
		}

		else if constexpr (std::is_same_v<T, detail::function_call>) {
			auto& f = detail::get_func(funcs::builtin_fn_id{x.f_id});
			return f.derivative(x.args, symbol.m_ref);
		}

		return expr.root;
	}, expr.root->p_data));
}
