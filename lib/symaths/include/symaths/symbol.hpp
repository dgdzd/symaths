#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include "symaths/expression.hpp"

#include <string>

namespace sym {
	expression differentiate(const expression& expr, const symbol& symbol);

	class symbol {
		friend expression;
		friend expression sym::differentiate(const expression& expr, const symbol& symbol);

	public:
		const detail::node* ref;

		symbol();
		symbol(const std::string& name);
		symbol(const char* name);
		symbol(const expression& expr);
		symbol(const detail::node* root);

		[[nodiscard]] const std::string& name() const;
		const std::string& name(const std::string& new_name);
	};
}

#endif
