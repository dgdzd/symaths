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

#ifndef SYM_PREDICATE_HPP
#define SYM_PREDICATE_HPP

#include "symaths/detail/predicate_node.hpp"

#include <string>

namespace sym {
	namespace detail {
		class predicate_node;
	}

	class predicate {
	public:
		const detail::predicate_node* root;

		predicate(const detail::predicate_node* node);

		bool operator()() const;
		[[nodiscard]] std::string string() const;

		/**
		 * @brief Checks if the predicate always evaluates to either true or false
		 *
		 * For example :
		 * - is_ground(3+ln(4) = sqrt(12)) == true
		 * - is_ground(3+4x = 2x^2) == false // DEPENDS ON "x"
		 *
		 * @return Whether the expression is ground
		 */
		[[nodiscard]] bool is_ground() const { return root->is_ground(); }

		[[nodiscard]] bool operator==(const predicate& other) const { return root == other.root; }
	};

	predicate operator&&(const predicate& lhs, const predicate& rhs);
	predicate operator||(const predicate& lhs, const predicate& rhs);
	predicate operator!(const predicate& lhs);
}

#endif
