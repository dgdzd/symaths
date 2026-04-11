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

#ifndef MATHS_H
#define MATHS_H

#include <array>

namespace sym::utils {
	static std::array<unsigned long long, 17> fact_table = {
		1, // 0!
		1, // 1!
		2, // 2!
		6, // 3!
		24, // 4!
		120, // 5!
		720, // 6!
		5040, // 7!
		40320, // 8!
		362880, // 9!
		3628800, // 10!
		39916800, // 11!
		479001600, // 12!
		6227020800, // 13!
		87178291200, // 14!
		1307674368000, // 15!
		20922789888000, // 16!
	};

	unsigned long long factorial(unsigned long long n);
	unsigned long long newton_binomial_coefficient(unsigned long long k, unsigned long long n);
	bool is_integer(double value, double eps = 1e-12);
}

#endif
