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
#include "symaths/detail/predicate_node.hpp"
#include "symaths/detail/set_node.hpp"

#include <map>
#include <optional>

namespace sym {
	namespace detail {
		class expression_node;
	}

	class context_table_t {
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
		std::vector<std::shared_ptr<entry>> m_objects;
		std::map<std::string, std::weak_ptr<entry>> m_table;

	public:
		context_table_t() = default;

		void add_entry(const detail::expression_node* obj);
		void add_entry(const detail::predicate_node* obj);
		void add_entry(const detail::set_node* obj);
		void add_entry(std::shared_ptr<entry> p_entry);
		void add_named_entry(const std::string& name, std::shared_ptr<entry> p_entry);

		void remove_entry(const void* obj);
		void clear();

		[[nodiscard]] const std::vector<std::shared_ptr<entry>>& get_objects() const;
		[[nodiscard]] std::vector<sym::expression> get_expressions() const;
		[[nodiscard]] std::vector<sym::predicate> get_predicates() const;
		[[nodiscard]] std::vector<sym::set> get_sets() const;

		[[nodiscard]] std::optional<sym::expression> find_expression(const std::string& name);
		[[nodiscard]] std::optional<sym::predicate> find_predicate(const std::string& name);
		[[nodiscard]] std::optional<sym::set> find_set(const std::string& name);
	};
}

#endif
