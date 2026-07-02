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

#ifndef SYM_COMPILE_HPP
#define SYM_COMPILE_HPP

#include <vector>

namespace sym {
	namespace detail {
		enum opcode : uint8_t {
			push_cst, push_var, call_fun,
			neg, add, sub, mul, div, pow,
		};

		struct instruction {
			opcode op;
			union {
				double val;
				size_t var_id;
				double (*fn)(const double*);
			};
		};
	}

	class compiled_expression {

	};
}

#endif
