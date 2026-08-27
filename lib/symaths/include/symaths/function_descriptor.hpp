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

#ifndef SYM_FUNCTION_DESCRIPTOR_HPP
#define SYM_FUNCTION_DESCRIPTOR_HPP

#include "symaths/detail/types.hpp"

#include <functional>

namespace sym {
	class context_table_t;

	template<typename T>
	using func_handler_t = std::function<T(
		const std::vector<detail::expression_value_t>&,
		context_table_t*
	)>;
	using args_list_t = std::vector<detail::value_type_t>;

	template<detail::is_object T>
	class function_descriptor {
		func_handler_t<T> m_handler;
		std::string m_name;
		std::vector<args_list_t> m_args;

	public:
		function_descriptor() = default;
		explicit function_descriptor(const std::string& name) : m_name(name) {}

		detail::value_type_t return_type() const;

		function_descriptor& with_name(const std::string& name);
		function_descriptor& with_handler(const func_handler_t<T>& handler);
		function_descriptor& add_signature(const args_list_t& args);
	};

	namespace detail {
		using internal_func_handler_t = std::function<object(
		const std::vector<expression_value_t>&,
			context_table_t*
		)>;

		struct internal_func_descriptor {
			const char* name;
			internal_func_handler_t handler;
			std::vector<args_list_t> arg_types;
			value_type_t return_type = null;
		};
	}

	template<detail::is_object T>
	void register_function_descriptor(const function_descriptor<T>& desc);
}

#endif