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
#include "symaths/function_descriptor.hpp"
#include "symaths/predicate.hpp"
#include "symaths/set.hpp"
#include "symaths/detail/object.hpp"
#include "symaths/detail/types.hpp"

#include <iostream>
#include <map>
#include <optional>

namespace sym {
	namespace detail {
		class mathexpr_node;
	}

	class context_table_t {
	public:
		struct entry {
			detail::value_type_t type = detail::null;
			object value;
		};

	private:
		std::vector<detail::internal_func_descriptor> m_funcs;
		std::map<std::string, entry> m_table;
		std::ostream* m_out = &std::cout;

	public:
		context_table_t();

		[[nodiscard]] std::ostream* out_stream() const;
		[[nodiscard]] std::ostream*& out_stream();
		void register_func(const detail::internal_func_descriptor& desc);
		void add_entry(const std::string& varname, detail::expression_value_t entry);
		void add_uninitialized_entry(const std::string& varname, detail::value_type_t type);
		void remove_entry(const std::string& varname);
		void clear();

		[[nodiscard]] uint32_t get_func_id(const std::string& name) const;
		[[nodiscard]] uint32_t func_count() const;
		[[nodiscard]] const detail::internal_func_descriptor& get_func(uint32_t id) const;
		[[nodiscard]] std::vector<args_list_t> get_candidates(const detail::internal_func_descriptor& func, size_t arguments_count) const;

		[[nodiscard]] std::optional<object> find(const std::string& name);
		[[nodiscard]] std::optional<bool> find_bool(const std::string& name);
		[[nodiscard]] std::optional<number> find_number(const std::string& name);
		[[nodiscard]] std::optional<expression> find_expression(const std::string& name);
		[[nodiscard]] std::optional<predicate> find_predicate(const std::string& name);
		[[nodiscard]] std::optional<set> find_set(const std::string& name);
	};
}

#endif
