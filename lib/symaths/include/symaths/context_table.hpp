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

#ifndef SYM_CONTEXT_TABLE_HPP
#define SYM_CONTEXT_TABLE_HPP

#include "symaths/expression.hpp"
#include "symaths/predicate.hpp"
#include "symaths/set.hpp"
#include "symaths/detail/types.hpp"

#include <map>
#include <optional>

namespace sym {
	namespace detail {
		class mathexpr_node;
	}

	class context_table_t {
		std::map<std::string, detail::expression_value_t> m_table;

	public:
		context_table_t() = default;

		void add_named_entry(const std::string& varname, detail::expression_value_t entry);
		void remove_entry(const std::string& varname);
		void clear();

		[[nodiscard]] std::optional<expression> find_expression(const std::string& name);
		[[nodiscard]] std::optional<predicate> find_predicate(const std::string& name);
		[[nodiscard]] std::optional<set> find_set(const std::string& name);
		[[nodiscard]] std::optional<number> find_number(const std::string& name);
		[[nodiscard]] std::optional<bool> find_bool(const std::string& name);
	};
}

#endif
