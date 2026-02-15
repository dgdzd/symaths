#include "symaths/expressions_manip.hpp"

#include "symaths/symaths.hpp"

sym::expression sym::reduce(const expression& expr) {
	return sort(std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication> ||  std::is_same_v<T, detail::power> || std::is_same_v<T, detail::negation>) {
			return x.reduced();
		}
		return expr.root;
	}, expr.root->p_data));
}

sym::expression sym::sort(const expression& expr) {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication> || std::is_same_v<T, detail::negation>) {
			return x.sorted();
		}

		return expr.root;
	}, expr.root->p_data);
}

sym::expression sym::expand(const expression& expr) {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication> || std::is_same_v<T, detail::power> || std::is_same_v<T, detail::negation>) {
			return x.expanded();
		}
		return expr.root;
	}, expr.root->p_data);
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

sym::detail::term sym::detail::extract_term(const node* node) {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		term t{};
		if constexpr (std::is_same_v<T, multiplication>) {
			t.coefficient = 1.0;
			std::vector<const detail::node*> final_operands;
			for (auto& op : x.operands) {
				if (op->is_ground()) {
					t.coefficient *= op->eval(nullptr);
				}
				else {
					final_operands.push_back(op);
				}
			}

			if (final_operands.size() == 1) {
				t.symbolic = final_operands.front();
			}
			else {
				t.symbolic = current_context->node_manager().make_mul(final_operands);
			}
		}
		else if constexpr (std::is_same_v<T, negation>) {
			t = extract_term(x.child);
			t.coefficient = -t.coefficient;
		}
		else {
			t.coefficient = 1.0;
			t.symbolic = node;
		}

		return t;
	}, node->p_data);
}

/*sym::detail::AdditionNodePtr sym::detail::develop(MultiplicationNodePtr node) {

}*/

