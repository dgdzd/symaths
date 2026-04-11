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

#ifndef INTERVAL_HPP
#define INTERVAL_HPP

namespace sym {
	namespace detail {
		class node;
	}

	class interval {
		const detail::node* lower = nullptr;
		const detail::node* upper = nullptr;
		bool lower_included = true;
		bool upper_included = true;
	};
}

#endif
