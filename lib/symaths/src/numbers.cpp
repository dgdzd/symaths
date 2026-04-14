#include "symaths/numbers.hpp"

#include "symaths/utils/maths.hpp"

#include <numeric>

using namespace sym;

double numbers::rational::double_value() const {
	return static_cast<double>(num) / static_cast<double>(den);
}

numbers::rational numbers::rational::simplified() const {
	auto gcd = std::gcd(num, den);
	return rational{num / gcd, den / gcd};
}


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
			auto q = n.simplified();
			if (std::abs(q.den) == 1) {
				auto p = q.den * q.num;
				if (p > 0) {
					return numbers::natural(p);
				}
				return numbers::integer{p};
			}
			return q;
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
			if (std::abs(z.val.imag()) < 1e-12) {
				auto n = numbers::real{z.val.real()};
				if (std::abs(n.val - std::round(n.val)) < 1e-12) {
					if (n.val >= 0) {
						return numbers::natural{static_cast<unsigned long long>(n.val)};
					}
					return numbers::integer{static_cast<long long>(n.val)};
				}
				return n;
			}
			return z;
		},
		[&](const auto& x) -> internal_data_t { return x; }
	}, p_data);
}

std::string number::string() const {
	return std::visit(overloaded {
		[&](const numbers::natural n) -> std::string { return std::format("{}", n.val); },
		[&](const numbers::integer n) -> std::string { return std::format("{}", n.val); },
		[&](const numbers::rational n) -> std::string { return std::format("{}/{}", n.num, n.den); },
		[&](const numbers::real n) -> std::string { return std::format("{}", n.val); },
		[&](const numbers::complex n) -> std::string {
			if (n.val.imag() < 0) {
				return std::format("{}{}i", n.val.real(), n.val.imag());
			}
			return std::format("{}+{}i", n.val.real(), n.val.imag());
		},
		[&](const numbers::nan&) -> std::string { return "nan"; }
	}, p_data);
}

number::rank number::get_rank(internal_data_t data) {
	return static_cast<rank>(data.index());
}

number::internal_data_t number::promote_to(internal_data_t data, rank rnk) {
	if (rnk == rank::NaN) {
		return numbers::nan{};
	}
	while (get_rank(data) < rnk) {
		std::visit(overloaded {
			[&](const numbers::natural& n) { data = numbers::integer(n.val); },
			[&](const numbers::integer& n) { data = numbers::rational(n.val, 1); },
			[&](const numbers::rational& q) { data = numbers::real(static_cast<double>(q.num) / static_cast<double>(q.den)); },
			[&](const numbers::real& r) { data = numbers::complex({r.val, 0}); },
			[&](const numbers::complex& z) { data = z; },
			[&](const auto&) { throw std::runtime_error("Invalid type for number."); }
		}, data);
	}
	return data;
}

bool number::operator==(const number& rhs) const {
	return p_data == rhs.p_data;
}

number::operator std::string() const {
	return string();
}


number sym::operator+(const number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	number::internal_data_t d1 = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	number num = std::visit(overloaded {
		[&](const numbers::natural& n1, const numbers::natural& n2)   -> number::internal_data_t { return numbers::natural{n1.val + n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2)   -> number::internal_data_t { return numbers::integer{n1.val + n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) -> number::internal_data_t { return numbers::rational{n1.num * n2.den + n2.num * n1.den, n1.den * n2.den}; },
		[&](const numbers::real& n1, const numbers::real& n2)         -> number::internal_data_t { return numbers::real{n1.val + n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2)   -> number::internal_data_t { return numbers::complex{n1.val + n2.val}; },
		[&](const numbers::nan& n1, const numbers::nan& n2)           -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&, const auto&)                                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
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
		[&](const numbers::natural& n1, const numbers::natural& n2)   -> number::internal_data_t { return numbers::natural{n1.val - n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2)   -> number::internal_data_t { return numbers::integer{n1.val - n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) -> number::internal_data_t { return numbers::rational{n1.num * n2.den - n2.num * n1.den, n1.den * n2.den}; },
		[&](const numbers::real& n1, const numbers::real& n2)         -> number::internal_data_t { return numbers::real{n1.val - n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2)   -> number::internal_data_t { return numbers::complex{n1.val - n2.val}; },
		[&](const numbers::nan& n1, const numbers::nan& n2)           -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&, const auto&)                                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
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
		[&](const numbers::natural& n1, const numbers::natural& n2)   -> number::internal_data_t { return numbers::natural{n1.val * n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2)   -> number::internal_data_t { return numbers::integer{n1.val * n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) -> number::internal_data_t { return numbers::rational{n1.num * n2.num, n1.den * n2.den}; },
		[&](const numbers::real& n1, const numbers::real& n2)         -> number::internal_data_t { return numbers::real{n1.val * n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2)   -> number::internal_data_t { return numbers::complex{n1.val * n2.val}; },
		[&](const numbers::nan& n1, const numbers::nan& n2)           -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&, const auto&)                                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
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
		[&](const numbers::natural& n1, const numbers::natural& n2)   -> number::internal_data_t { return numbers::rational{n1.val, n2.val}; },
		[&](const numbers::integer& n1, const numbers::integer& n2)   -> number::internal_data_t { return numbers::rational{n1.val, n2.val}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) -> number::internal_data_t { return numbers::rational{n1.num * n2.den, n1.den * n2.num}; },
		[&](const numbers::real& n1, const numbers::real& n2)         -> number::internal_data_t { return numbers::real{n1.val / n2.val}; },
		[&](const numbers::complex& n1, const numbers::complex& n2)   -> number::internal_data_t { return numbers::complex{n1.val / n2.val}; },
		[&](const numbers::nan& n1, const numbers::nan& n2)           -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&, const auto&)                                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
	}, d1, d2);
	num.downcast();
	return num;
}

number sym::operator-(const number& n) {
	number num = std::visit(overloaded {
		[&](const numbers::natural& n1)  -> number::internal_data_t { return numbers::integer{-static_cast<long long>(n1.val)}; },
		[&](const numbers::integer& n1)  -> number::internal_data_t { return numbers::integer{-n1.val}; },
		[&](const numbers::rational& n1) -> number::internal_data_t { return numbers::rational{-n1.num, n1.den}; },
		[&](const numbers::real& n1)     -> number::internal_data_t { return numbers::real{-n1.val}; },
		[&](const numbers::complex& n1)  -> number::internal_data_t { return numbers::complex{-n1.val}; },
		[&](const numbers::nan& n1)      -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&)                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
	}, n.p_data);
	num.downcast();
	return num;
}

number& sym::operator+=(number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	lhs.p_data = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	std::visit(overloaded {
		[&](numbers::natural& n1, const numbers::natural& n2) { n1.val += n2.val; },
		[&](numbers::integer& n1, const numbers::integer& n2) { n1.val += n2.val; },
		[&](numbers::rational& n1, const numbers::rational& n2) {
			n1.num = n1.num * n2.den + n2.num * n1.den;
			n1.den = n1.den * n2.den;
		},
		[&](numbers::real& n1, const numbers::real& n2) { n1.val += n2.val; },
		[&](numbers::complex& n1, const numbers::complex& n2) { n1.val += n2.val; },
		[&](numbers::nan& n1, const numbers::nan& n2) { return; },
		[&](auto&, const auto&) { throw std::runtime_error("Type mismatch."); },
	}, lhs.p_data, d2);
	lhs.downcast();
	return lhs;
}

number& sym::operator-=(number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	lhs.p_data = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	std::visit(overloaded {
		[&](numbers::natural& n1, const numbers::natural& n2) { n1.val -= n2.val; },
		[&](numbers::integer& n1, const numbers::integer& n2) { n1.val -= n2.val; },
		[&](numbers::rational& n1, const numbers::rational& n2) {
			n1.num = n1.num * n2.den - n2.num * n1.den;
			n1.den = n1.den * n2.den;
		},
		[&](numbers::real& n1, const numbers::real& n2) { n1.val -= n2.val; },
		[&](numbers::complex& n1, const numbers::complex& n2) { n1.val -= n2.val; },
		[&](numbers::nan& n1, const numbers::nan& n2) { return; },
		[&](auto&, const auto&) { throw std::runtime_error("Type mismatch."); },
	}, lhs.p_data, d2);
	lhs.downcast();
	return lhs;
}

number& sym::operator*=(number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	lhs.p_data = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	std::visit(overloaded {
		[&](numbers::natural& n1, const numbers::natural& n2) { n1.val *= n2.val; },
		[&](numbers::integer& n1, const numbers::integer& n2) { n1.val *= n2.val; },
		[&](numbers::rational& n1, const numbers::rational& n2) {
			n1.num *= n2.num;
			n1.den *= n2.den;
		},
		[&](numbers::real& n1, const numbers::real& n2) { n1.val *= n2.val; },
		[&](numbers::complex& n1, const numbers::complex& n2) { n1.val *= n2.val; },
		[&](numbers::nan& n1, const numbers::nan& n2) { return; },
		[&](auto&, const auto&) { throw std::runtime_error("Type mismatch."); },
	}, lhs.p_data, d2);
	lhs.downcast();
	return lhs;
}

number& sym::operator/=(number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	lhs.p_data = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	std::visit(overloaded {
		[&](numbers::natural& n1, const numbers::natural& n2) { lhs.p_data = numbers::rational{n1.val, n2.val}; },
		[&](numbers::integer& n1, const numbers::integer& n2) { lhs.p_data = numbers::rational{n1.val, n2.val}; },
		[&](numbers::rational& n1, const numbers::rational& n2) {
			n1.num *= n2.den;
			n1.den *= n2.num;
		},
		[&](numbers::real& n1, const numbers::real& n2) { n1.val /= n2.val; },
		[&](numbers::complex& n1, const numbers::complex& n2) { n1.val /= n2.val; },
		[&](numbers::nan& n1, const numbers::nan& n2) { return; },
		[&](auto&, const auto&) { throw std::runtime_error("Type mismatch."); },
	}, lhs.p_data, d2);
	lhs.downcast();
	return lhs;
}


number sym::pow_calc(const number& lhs, const number& rhs) {
	auto r1 = number::get_rank(lhs.p_data);
	auto r2 = number::get_rank(rhs.p_data);
	number::rank final_rank = std::max(r1, r2);
	number::internal_data_t d1 = number::promote_to(lhs.p_data, final_rank);
	number::internal_data_t d2 = number::promote_to(rhs.p_data, final_rank);

	number num = std::visit(overloaded {
		[&](const numbers::natural& n1, const numbers::natural& n2)   -> number::internal_data_t { return numbers::natural{static_cast<unsigned long long>(std::pow(n1.val, n2.val))}; },
		[&](const numbers::integer& n1, const numbers::integer& n2)   -> number::internal_data_t { return numbers::integer{static_cast<long long>(std::pow(n1.val, n2.val))}; },
		[&](const numbers::rational& n1, const numbers::rational& n2) -> number::internal_data_t { return numbers::real{std::pow(n1.double_value(), n2.double_value())}; },
		[&](const numbers::real& n1, const numbers::real& n2)         -> number::internal_data_t { return numbers::real{std::pow(n1.val, n2.val)}; },
		[&](const numbers::complex& n1, const numbers::complex& n2)   -> number::internal_data_t { return numbers::complex{std::pow(n1.val, n2.val)}; },
		[&](const numbers::nan& n1, const numbers::nan& n2)           -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&, const auto&)                                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
	}, d1, d2);
	num.downcast();
	return num;
}

number sym::abs_calc(const number& lhs) {
	number num = std::visit(overloaded {
		[&](const numbers::natural& n1)  -> number::internal_data_t { return n1; },
		[&](const numbers::integer& n1)  -> number::internal_data_t { return numbers::integer{std::abs(n1.val)}; },
		[&](const numbers::rational& n1) -> number::internal_data_t { return numbers::rational{std::abs(n1.num), std::abs(n1.den)}; },
		[&](const numbers::real& n1)     -> number::internal_data_t { return numbers::real{std::abs(n1.val)}; },
		[&](const numbers::complex& n1)  -> number::internal_data_t { return numbers::complex{std::abs(n1.val)}; },
		[&](const numbers::nan& n1)      -> number::internal_data_t { return numbers::nan{}; },
		[&](const auto&)                 -> number::internal_data_t { throw std::runtime_error("Type mismatch."); },
	}, lhs.p_data);
	num.downcast();
	return num;
}

