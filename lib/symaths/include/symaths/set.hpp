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

#ifndef SYM_SET_HPP
#define SYM_SET_HPP

#include "symaths/detail/set_node.hpp"

#include <string>

namespace sym {

	class set {
	public:
		const detail::set_node* root;

		set(const detail::set_node* node);
		set(double val);

		[[nodiscard]] std::string string() const;

		/**
		 * @brief Checks if the expression is composed of constants only
		 *
		 * For example :
		 * - is_ground(3 + ln(4)) == true
		 * - is_ground(3 + 4x) == false
		 *
		 * @return Whether the expression is ground
		 */
		[[nodiscard]] bool is_ground() const { return false; } // TODO

		[[nodiscard]] bool operator==(const set& other) const { return root == other.root; }
	};

	set operator|(const set& lhs, const set& rhs);
	set operator&(const set& lhs, const set& rhs);
	set operator!(const set& e);
}

#endif
