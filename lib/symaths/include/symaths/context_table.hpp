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
#include "symaths/detail/predicate_node.hpp"
#include "symaths/detail/set_node.hpp"

#include <optional>

namespace sym {
	namespace detail {
		class expression_node;
	}

	class context_table {
	public:
		enum obj_type {
			NONE = -1,
			expression,
			predicate,
			set,
			function,
		};
		struct entry {
			obj_type type;
			const void* value;
		};

	private:
		std::vector<entry> m_objects;

	public:
		context_table() = default;

		void add_entry(const detail::expression_node* obj);
		void add_entry(const detail::predicate_node* obj);
		void add_entry(const detail::set_node* obj);

		void remove_entry(const void* obj);
		void clear();

		[[nodiscard]] const std::vector<entry>& get_objects() const;
		[[nodiscard]] std::vector<sym::expression> get_expressions() const;
		[[nodiscard]] std::vector<detail::predicate_node> get_predicates() const;
		[[nodiscard]] std::vector<detail::set_node> get_sets() const;

		[[nodiscard]] std::optional<detail::predicate_node> find_predicate(const std::string& name) const;
		[[nodiscard]] std::optional<detail::set_node> find_set(const std::string& name) const;
	};
}

#endif
