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

#ifndef SYM_NODE_SET_HPP
#define SYM_NODE_SET_HPP

#include "symaths/interval.hpp"

#include <variant>
#include <vector>

namespace sym {
	/*
	 * First what is a set ? A set is :
	 * - An association of intervals
	 * - A list of numbers which satisfy conditions (for example : N = set of all positive integers)
	 *
	 * So we have continuous intervals (like R, Q or C) and discrete intervals (like N or Z).
	 * Sets can also be disjoint, like R* = ]-∞;0[ ∪ ]0;+∞[
	 */


	namespace detail {
		class set_node;
	}

	namespace detail {
		struct continuous_set {
			std::vector<interval> disjoint_intervals;

			bool operator==(const continuous_set&) const = default;
		};

		// No need to store any information (integers are human construct and convention)
		struct integer_set {

			bool operator==(const integer_set&) const = default;
		};

		struct conditional_set {
			const set_node* base_set;
			const expression_node* symbol;

			bool operator==(const conditional_set&) const = default;
		};

		struct intersection_set {
			std::vector<const set_node*> sets;

			bool operator==(const intersection_set&) const = default;
		};

		class set_node {
		public:
			using internal_data_t = std::variant<continuous_set, integer_set, conditional_set, intersection_set>;

			internal_data_t p_data;
			size_t p_hash = 0;

			bool contains();
		};
	}
}

#endif
