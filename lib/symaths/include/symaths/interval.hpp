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

#ifndef SYM_INTERVAL_HPP
#define SYM_INTERVAL_HPP

namespace sym {
	namespace detail {
		class mathexpr_node;
	}

	class interval {
		const detail::mathexpr_node* lower = nullptr;
		const detail::mathexpr_node* upper = nullptr;
		bool lower_included = true;
		bool upper_included = true;

	public:
		bool operator==(const interval&) const = default;
	};
}

#endif
