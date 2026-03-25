#ifndef EQUATION_HPP
#define EQUATION_HPP

#include "symaths/expression.hpp"

namespace sym {
	class equation {
		expression m_left;
		expression m_right;

	public:
		equation(const expression& left, const expression& right) : m_left(left), m_right(right) {}

		const expression& left() const { return m_left; }
		expression& left() { return m_left; }
		const expression& right() const { return m_right; }
		expression& right() { return m_right; }

		std::vector<const symbol&> unknowns();
		bool is_solvable();

		std::vector<expression> solve() const;
	};
}

#endif
