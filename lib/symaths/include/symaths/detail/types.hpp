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

#ifndef SYM_TYPES_HPP
#define SYM_TYPES_HPP

#include "symaths/numbers.hpp"
#include "symaths/utils/exceptions.hpp"

namespace sym::detail {
	class mathexpr_node;
	class predicate_node;
	class set_node;

	/*
	 * Expression is NOT mathexpr. Expression is a value in the program's AST. Mathexpr is a type in the symaths language,
	 * a kind of expression.
	 */
	using expression_value_t = std::variant<bool, number, exception,
		const mathexpr_node*, const predicate_node*, const set_node*>;

	enum value_type_t : int8_t {
		null = -1,
		bool_ = variant_index<expression_value_t, bool>(),
		number_ = variant_index<expression_value_t, number>(),
		exception_ = variant_index<expression_value_t, exception>(),
		mathexpr_ = variant_index<expression_value_t, const mathexpr_node*>(),
		predicate_ = variant_index<expression_value_t, const predicate_node*>(),
		set_ = variant_index<expression_value_t, const set_node*>(),
		LEN
	};

	template <typename T>
	concept is_symaths_object = requires(T t)
	{
		requires variant_index<expression_value_t, decltype(T::root)>() != value_type_t::LEN;
	};

	inline std::string value_type_string(value_type_t type) {
		switch (type) {
			case null:
				return "null";
			case bool_:
				return "bool";
			case number_:
				return "number";
			case exception_:
				return "exception";
			case mathexpr_:
				return "mathexpr";
			case predicate_:
				return "predicate";
			case set_:
				return "set";
			default:
				return "<unknown>";
		}
	}
}

#endif
