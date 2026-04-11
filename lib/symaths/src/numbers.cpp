#include "symaths/numbers.hpp"

#include "symaths/utils/helpers.hpp"

using namespace sym;

void number::downcast() {
	p_data = std::visit(overloaded {
		[&](const numbers::natural& n) -> internal_data_t { return n; },
		[&](const numbers::integer& n) -> internal_data_t {
			if (n.val >= 0) {
				return numbers::natural{static_cast<unsigned long long>(n.val)};
			}
			return n;
		},
		[&](const numbers::rational& n) -> internal_data_t {
			if (std::abs(n.den) == 1) {
				auto p = n.den * n.num;
				if (p > 0) {
					return numbers::natural(p);
				}
				return numbers::integer{p};
			}
			return n;
		},
		[&](const numbers::real& n) -> internal_data_t {
			if (std::abs(n.val - std::round(n.val)) < 1e-12) {
				if (n.val >= 0) {
					return numbers::natural{static_cast<unsigned long long>(n.val)};
				}
				return numbers::integer{static_cast<long long>(n.val)};
			}
			return n;
		},
		[&](const numbers::complex& z) -> internal_data_t {
			if (std::abs(z.val.imag) < 1e-12) {
				auto n = numbers::real{z.val.real};
				if (std::abs(n.val - std::round(n.val)) < 1e-12) {
					if (n.val >= 0) {
						return numbers::natural{static_cast<unsigned long long>(n.val)};
					}
					return numbers::integer{static_cast<long long>(n.val)};
				}
				return n;
			}
			return z;
		}
	}, p_data);
}

number::rank number::get_rank(internal_data_t data) {
	return static_cast<rank>(data.index());
}

number::internal_data_t number::promote_to(internal_data_t data, rank rnk) {
	while (get_rank(data) < rnk) {
		std::visit(overloaded {
			[&](const numbers::natural& n) { data = numbers::integer(static_cast<long long>(n.val)); },
			[&](const numbers::integer& n) { data = numbers::rational(static_cast<long long>(n.val)); },
			[&](const numbers::rational& q) { data = numbers::real(static_cast<double>(q.num) / static_cast<double>(q.den)); },
			[&](const numbers::real& r) { data = numbers::complex({r.val, 0}); },
			[&](const numbers::complex& z) { data = z; }
		}, data);
	}
	return data;
}


number sym::operator+(const number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	number::internal_data_t d1 = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	number num = std::visit(overloaded {
		[&](const numbers::natural& n1, const numbers::natural& n2) { return numbers::natural{n1.val + n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2) { return numbers::integer{n1.val + n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) { return numbers::rational{n1.num * n2.den + n2.num * n1.den, n1.den * n2.den}; },
		[&](const numbers::real& n1, const numbers::real& n2) { return numbers::real{n1.val + n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2) { return numbers::complex{n1.val + n2.val}; },
	}, d1, d2);
	num.downcast();
	return num;
}

number sym::operator-(const number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	number::internal_data_t d1 = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	number num = std::visit(overloaded {
		[&](const numbers::natural& n1, const numbers::natural& n2) { return numbers::natural{n1.val - n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2) { return numbers::integer{n1.val - n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) { return numbers::rational{n1.num * n2.den - n2.num * n1.den, n1.den * n2.den}; },
		[&](const numbers::real& n1, const numbers::real& n2) { return numbers::real{n1.val - n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2) { return numbers::complex{n1.val - n2.val}; },
	}, d1, d2);
	num.downcast();
	return num;
}

number sym::operator*(const number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	number::internal_data_t d1 = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	number num = std::visit(overloaded {
		[&](const numbers::natural& n1, const numbers::natural& n2) { return numbers::natural{n1.val * n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2) { return numbers::integer{n1.val * n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) { return numbers::rational{n1.num * n2.num, n1.den * n2.den}; },
		[&](const numbers::real& n1, const numbers::real& n2) { return numbers::real{n1.val * n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2) { return numbers::complex{n1.val * n2.val}; },
	}, d1, d2);
	num.downcast();
	return num;
}

number sym::operator/(const number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	number::internal_data_t d1 = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	number num = std::visit(overloaded {
		[&](const numbers::natural& n1, const numbers::natural& n2) { return numbers::rational{static_cast<long long>(n1.val), static_cast<long long>(n2.val)}; },
		[&](const numbers::integer& n1, const numbers::integer& n2) { return numbers::rational{n1.val, n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) { return numbers::rational{n1.num * n2.den, n1.den * n2.num}; },
		[&](const numbers::real& n1, const numbers::real& n2) { return numbers::real{n1.val / n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2) { return numbers::complex{n1.val / n2.val}; },
	}, d1, d2);
	num.downcast();
	return num;
}

number sym::operator-(const number& n) {
	number num = std::visit(overloaded {
		[&](const numbers::natural& n1) { return numbers::integer{-static_cast<long long>(n1.val)}; },
		[&](const numbers::integer& n1) { return numbers::integer{-n1.val}; },
		[&](const numbers::rational& n1) { return numbers::rational{-n1.num, n1.den}; },
		[&](const numbers::real& n1) { return numbers::real{-n1.val}; },
		[&](const numbers::complex& n1) { return numbers::complex{-n1.val}; },
	}, n.p_data);
	num.downcast();
	return num;
}