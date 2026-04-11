/*
 *	                            _   _
 *	  ___ _   _ _ __ ___   __ _| |_| |__  ___
 *	 / __| | | | '_ ` _ \ / _` | __| '_ \/ __|   Symbolic maths for C++
 *	 \__ \ |_| | | | | | | (_| | |_| | | \__ \   Version : 0.0.1
 *	 |___/\__, |_| |_| |_|\__,_|\__|_| |_|___/   https://github.com/dgdzd/symaths
 *		  |___/
 *
 * All source code is distributed under the GNU General Public License v2.0.
 *
 */

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
