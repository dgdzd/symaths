#include "symaths/base_operations.hpp"

#include <algorithm>

#include "symaths/expressions_manip.hpp"

#include <map>

namespace sym::detail {
	void n_operation::add_operand(NodePtr operand) {
		operands.push_back(operand);
	}

	NodePtr n_operation::operator[](unsigned int index) const {
		return operands.at(index);
	}
}

namespace sym::objs {
	double variable::eval(const detail::Context* ctx) const {
		return ctx->at(m_name); // Throws if variable not found
	}

	std::string variable::string(const node* parent) const {
		return m_name;
	}

	bool variable::is_ground() const {
		return false;
	}

	const std::string& variable::name() const {
		return m_name;
	}

	double constant::eval(const detail::Context* ctx) const {
		return value;
	}

	std::string constant::string(const node* parent) const {
		return std::format("{}", value);
	}

	bool constant::is_ground() const {
		return true;
	}

	double addition::eval(const detail::Context* ctx) const {
		double sum = 0.0;
		for (const auto& op : operands) {
			sum += op->eval(ctx);
		}
		return sum;
	}

	std::string addition::string(const node* parent) const {
		bool par = false;
		std::string s;
		if (parent && parent->priority() > priority()) par = true;

		if (par) s += "(";
		for (size_t i = 0; i < operands.size(); ++i) {
			s += operands[i]->string(this);
			if (i < operands.size() - 1) s += " + ";
		}
		if (par) s += ")";
		return s;
	}

	bool addition::is_ground() const {
		return std::ranges::all_of(operands, [](const auto& op) {
			return op->is_ground();
		});
	}

	void addition::flatten() {
		for (auto it = operands.begin(); it != operands.end(); ++it) {
			auto& op = *it;
			if (auto add = std::dynamic_pointer_cast<addition>(op)) {
				add->flatten();
				operands.erase(it);
				operands.append_range(add->get_operands());
			}
		}
	}

	const std::vector<detail::NodePtr>& addition::get_operands() const {
		return operands;
	}

	detail::NodePtr addition::reduce() {
		// In case of other sub-expressions, recursively simplify them
		// Hold each variable and its total coefficient
		std::map<std::string, double> m;

		// Get all variables' coefficient
		for (auto& op : operands) {

			// If the operand is a constant
			if (op->is_ground()) {
				if (m.contains("")) {
					m[""] += op->eval(nullptr);
				}
				else {
					m[""] = op->eval(nullptr);
				}
			}
			// Else if the operand is not constant, separate constant part and variable part
			else if (auto mul = std::dynamic_pointer_cast<multiplication>(op)) {
				auto term_ = detail::extract_term(mul);
				if (m.contains)
			}
		}
	}

	double negate::eval(const detail::Context* ctx) const {
		return -child->eval(ctx);
	}

	std::string negate::string(const node* parent) const {
		std::string s;
		if (parent && parent->priority() > priority())
			return "(-" + child->string(this) + ")";

		return "-" + child->string(this);

	}

	bool negate::is_ground() const {
		return child->is_ground();
	}

	detail::NodePtr negate::get_child() const {
		return child;
	}

	double multiplication::eval(const detail::Context* ctx) const {
		double m = 1.0;
		for (const auto& op : operands) {
			m *= op->eval(ctx);
		}
		return m;
	}

	std::string multiplication::string(const node* parent) const {
		bool par = false;
		std::string s;
		if (parent && parent->priority() > priority()) par = true;

		if (par) s = "(";
		for (size_t i = 0; i < operands.size(); ++i) {
			s += operands[i]->string(this);
			if (i < operands.size() - 1) s += " * ";
		}
		if (par) s += ")";
		return s;
	}

	bool multiplication::is_ground() const {
		return std::ranges::all_of(operands, [](const auto& op) {
			return op->is_ground();
		});
	}

	void multiplication::flatten() {
		for (auto it = operands.begin(); it != operands.end(); ++it) {
			auto& op = *it;
			if (auto add = std::dynamic_pointer_cast<multiplication>(op)) {
				add->flatten();
				operands.erase(it);
				operands.append_range(add->get_operands());
			}
		}
	}

	const std::vector<detail::NodePtr>& multiplication::get_operands() const {
		return operands;
	}

	detail::AdditionNodePtr multiplication::unroll() const {

	}

	double division::eval(const detail::Context* ctx) const {
		double m = 1.0;
		for (const auto& op : operands) {
			m /= op->eval(ctx);
		}
		return m;
	}

	std::string division::string(const node* parent) const {
		bool par = false;
		std::string s;
		if (parent && parent->priority() > priority()) par = true;

		if (par) s = "(";
		for (size_t i = 0; i < operands.size(); ++i) {
			s += operands[i]->string(this);
			if (i < operands.size() - 1) s += " / ";
		}
		if (par) s += ")";
		return s;
	}

	bool division::is_ground() const {
		return std::ranges::all_of(operands, [](const auto& op) {
			return op->is_ground();
		});
	}

	void division::flatten() {
		for (auto it = operands.begin(); it != operands.end(); ++it) {
			auto& op = *it;
			if (auto add = std::dynamic_pointer_cast<division>(op)) {
				add->flatten();
				operands.erase(it);
				operands.append_range(add->get_operands());
			}
		}
	}

	const std::vector<detail::NodePtr>& division::get_operands() const {
		return operands;
	}

	double power::eval(const detail::Context* ctx) const {
		return std::pow(m_base->eval(ctx), m_exp->eval(ctx));
	}

	std::string power::string(const node* parent) const {
		std::string s;
		if (parent && parent->priority() > priority())
			return "(" + m_base->string(this) + "^" + m_exp->string(this) + ")";

		return m_base->string(this) + "^" + m_exp->string(this);
	}

	bool power::is_ground() const {
		return m_base->is_ground() && m_exp->is_ground();
	}

	const detail::NodePtr& power::base() const {
		return m_base;
	}

	const detail::NodePtr& power::exponent() const {
		return m_exp;
	}
}
