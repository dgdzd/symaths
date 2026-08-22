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

#ifndef SYM_OBJECT_HPP
#define SYM_OBJECT_HPP

#include "symaths/detail/types.hpp"

#include <optional>

namespace sym {
	class program;

	// Friendly wrapper around std::optional<detail::expression_value_t>
	class object {
	public:
		std::optional<detail::expression_value_t> root;

		object() = default;
		explicit object(const std::optional<detail::expression_value_t>& root) : root(root) {}

		template <detail::is_symaths_object T>
		T cast() const {
			if (root.has_value()) {
				return T(std::get<decltype(T::root)>(root.value()));
			}
			throw std::logic_error{"Cannot cast null program_output."};
		}

		template <typename T>
		T cast() const {
			if (root.has_value()) {
				return std::get<T>(root.value());
			}
			throw std::logic_error{"Cannot cast null program_output."};
		}

		operator bool() const;
		bool operator!() const;
		[[nodiscard]] std::string string() const;
		[[nodiscard]] detail::value_type_t type() const;
	};
}

#endif
