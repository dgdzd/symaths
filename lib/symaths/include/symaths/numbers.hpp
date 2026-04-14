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

#include "symaths/utils/helpers.hpp"

#include <complex>
#include <variant>

namespace sym {
	namespace numbers {
		struct nan {
			bool operator==(const nan&) const = default;
		};

		struct natural {
			unsigned long long val;

			bool operator==(const natural&) const = default;
		};

		struct integer {
			long long val;

			bool operator==(const integer&) const = default;
		};

		struct rational {
			long long num;
			long long den;

			rational(long long num, long long den) : num(num), den(den) {}
			rational(unsigned long long num, unsigned long long den) : num(static_cast<long long>(num)), den(static_cast<long long>(den)) {}

			[[nodiscard]] double double_value() const;
			[[nodiscard]] rational simplified() const;
			bool operator==(const rational&) const = default;
		};

		struct real {
			double val;

			bool operator==(const real&) const = default;
		};

		struct complex {
			std::complex<double> val;

			bool operator==(const complex&) const = default;
		};
	}

	class number {
	public:
		enum class rank { Natural = 0, Integer = 1, Rational = 2, Real = 3, Complex = 4, NaN = 5 };

		// Helper to get rank from variant
		using internal_data_t = std::variant<numbers::natural, numbers::integer, numbers::rational, numbers::real, numbers::complex, numbers::nan>;

		internal_data_t p_data;

		number() : p_data(numbers::nan{}) {}
		number(internal_data_t x) : p_data(x) {}
		number(numbers::natural x) : p_data(x) {}
		number(numbers::integer x) : p_data(x) {}
		number(numbers::rational x) : p_data(x) {}
		number(numbers::real x) : p_data(x) {}
		number(numbers::complex x) : p_data(x) {}
		number(numbers::nan x) : p_data(x) {}

		void downcast();
		std::string string() const;

		bool operator==(const number& rhs) const;
		operator std::string() const;

		static rank get_rank(internal_data_t data);
		static internal_data_t promote_to(internal_data_t data, rank rnk);

		template<typename T>
		T get() const {
			return std::visit(overloaded {
				[&](const numbers::rational& x) -> T { return static_cast<T>(x.double_value()); },
				[&](const numbers::complex& x) -> T {
					if constexpr (std::is_same_v<T, std::complex<double>>) {
						return x.val;
					}
					else {
						return x.val.real();
					}
				},
				[&](const numbers::nan&) -> T { throw std::invalid_argument("Not a number."); },
				[&](const auto& x) -> T { return static_cast<T>(x.val); }
			}, p_data);
		}
	};

	number operator+(const number& lhs, const number& rhs);
	number operator-(const number& lhs, const number& rhs);
	number operator*(const number& lhs, const number& rhs);
	number operator/(const number& lhs, const number& rhs);
	number operator-(const number& n);

	number& operator+=(number& lhs, const number& rhs);
	number& operator-=(number& lhs, const number& rhs);
	number& operator*=(number& lhs, const number& rhs);
	number& operator/=(number& lhs, const number& rhs);

	number pow_calc(const number& lhs, const number& rhs);
	number abs_calc(const number& lhs);
}

#endif
