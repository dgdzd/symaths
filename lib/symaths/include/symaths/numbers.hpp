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

#ifndef NUMBER_HPP
#define NUMBER_HPP

#include <complex>
#include <variant>

namespace sym {
	namespace numbers {
		struct natural {
			unsigned long long val;
		};

		struct integer {
			long long val;
		};

		struct rational {
			long long num;
			long long den;

			void simplify();
		};

		struct real {
			double val;
		};

		struct complex {
			std::complex<double> val;
		};
	}

	class number {
	public:
		enum class rank { Natural = 0, Integer = 1, Rational = 2, Real = 3, Complex = 4 };

		// Helper to get rank from variant
		using internal_data_t = std::variant<numbers::natural, numbers::integer, numbers::rational, numbers::real, numbers::complex>;

		internal_data_t p_data;

		number(numbers::natural x);
		number(numbers::integer x);
		number(numbers::rational x);
		number(numbers::real x);
		number(numbers::complex x);

		void downcast();

		static rank get_rank(internal_data_t data);
		static internal_data_t promote_to(internal_data_t data, rank rnk);
	};

	number operator+(const number& lhs, const number& rhs);
	number operator-(const number& lhs, const number& rhs);
	number operator*(const number& lhs, const number& rhs);
	number operator/(const number& lhs, const number& rhs);
	number operator-(const number &n);
}

#endif
