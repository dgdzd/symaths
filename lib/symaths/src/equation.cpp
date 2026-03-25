#include "symaths/equation.hpp"

using namespace sym;

std::vector<const symbol&> equation::unknowns() {
	std::vector<const symbol&> symbols;
	expression eq = m_left - m_right;

	std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::symbol>) {
			if (std::ranges::find(symbols, eq) == symbols.end()) {
				symbols.push_back(eq);
			}
		}
	}, eq.root->p_data);
}
