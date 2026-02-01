#include "symaths/base_operations.hpp"

#include "symaths/fwd_decls.hpp"
#include "symaths/expressions_manip.hpp"

#include <algorithm>
#include <map>
#include <ranges>

namespace sym::detail {
	void n_operation::add_operand(NodePtr operand) {
		operands.push_back(operand);
	}

	NodePtr n_operation::operator[](unsigned int index) const {
		return operands.at(index);
	}

	const std::vector<NodePtr>& n_operation::get_operands() const {
		return operands;
	};
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
			if (i != 0) {
				std::string spaces(print_policies.sum.operand_spaces, ' ');
				s += spaces + "+";
			}
			s += operands[i]->string(this);
		}
		if (par) s += ")";
		return s;
	}

	bool addition::is_ground() const {
		return std::ranges::all_of(operands, [](const auto& op) {
			return op->is_ground();
		});
	}

	ptr<detail::n_operation> addition::sorted() const {
		// First sort itself, then sort its subexpressions
		ptr<addition> cpy = std::make_shared<addition>(*this);

		for (auto& op : cpy->operands) {
			if (auto operation = std::dynamic_pointer_cast<n_operation>(op)) {
				op = operation->sorted();
			}
		}
		std::ranges::sort(cpy->operands, [](const auto& op1, const auto& op2) {
			/* First sort value by type :
			 * 1. Values
			 * 2. Variables
			 * 3. Other subexprs
			 *
			 * If they have the same type, sort by power
			 * If they have the same power or their exponent is not a ground expression, sort by length
			 * If they have the same length, sort alphabetically
			 */
			bool gnd1 = op1->is_ground();
			bool gnd2 = op2->is_ground();
			if (gnd1 != gnd2) {
				return gnd2; // All ground values should be to the right
			}

			double pw1 = 1.0;
			double pw2 = 1.0;
			if (op1->kind() == kind_::power) {
				auto exp = std::dynamic_pointer_cast<power>(op1)->exponent();
				if (exp->is_ground()) pw1 = exp->eval(nullptr);
				else pw1 = std::numeric_limits<double>::max();
			}
			if (op2->kind() == kind_::power) {
				auto exp = std::dynamic_pointer_cast<power>(op2)->exponent();
				if (exp->is_ground()) pw2 = exp->eval(nullptr);
				else pw2 = std::numeric_limits<double>::max();
			}

			if (pw1 != pw2) return pw1 > pw2;

			size_t l1 = op1->string(nullptr).size();
			size_t l2 = op2->string(nullptr).size();
			if (l1 != l2) return l1 < l2;

			return op1->string(nullptr) < op2->string(nullptr);
		});

		return cpy;
	}

	void addition::flatten() {
		std::vector<std::pair<std::vector<detail::NodePtr>::iterator, ptr<addition>>> to_replace; // Holds iterator to replace
		for (auto it = operands.begin(); it != operands.end(); ++it) {
			auto& op = *it;
			if (auto add = std::dynamic_pointer_cast<addition>(op)) {
				add->flatten();
				to_replace.emplace_back(it, add); // Deleting or adding now invalidates vector iterators
			}
		}

		for (auto rit = to_replace.rbegin(); rit != to_replace.rend(); ++rit) {
			auto& [it, op] = *rit;
			unsigned int i = std::distance(operands.begin(), it);

			// First erase the old operand, and then insert its operands at its position
			operands.erase(it);
			operands.insert_range(operands.begin() + i, op->get_operands());
		}
	}

	detail::NodePtr addition::reduced() {
		// In case of other sub-expressions, recursively simplify them
		// Hold each variable and its total coefficient
		std::map<std::string, std::pair<double, detail::NodePtr>> m;

		// Get all variables' coefficient
		for (auto& op : operands) {

			// If the operand is a constant
			if (op->is_ground()) {
				if (m.contains("")) {
					m[""].first += op->eval(nullptr);
				}
				else {
					m[""] = {op->eval(nullptr), nullptr};
				}
			}
			// Else if the operand is not constant, separate constant part and variable part
			else {
				detail::term term_ = detail::extract_term(op);

				// Don't add term if coefficient is (almost) null
				if (std::abs(term_.coefficient) > 1e-9) {
					auto name = term_.symbolic->string(nullptr);
					if (m.contains(name)) {
						m[name].first += term_.coefficient;
					}
					else {
						m[name] = {term_.coefficient, term_.symbolic};
					}
				}
			}
		}

		// Now, create a new sorted branch

		if (m.size() <= 1) {
			if (m.begin()->first.empty()) {
				return std::make_shared<constant>(m.begin()->second.first);
			}

			return std::make_shared<multiplication>(
				std::make_shared<constant>(m.begin()->second.first),
				std::make_shared<variable>(m.begin()->first)
			);
		}

		ptr<addition> new_expr = std::make_shared<addition>();
		for (auto& [name, val] : m) {
			auto& [coeff, expr] = val;

			// Don't make a multiplication if it is just a constant
			if (name.empty()) {
				new_expr->add_operand(std::make_shared<constant>(coeff));
				continue;
			}

			// If it is (almost) equal to 1, don't create a multiplication
			if (std::abs(coeff - 1.0) < 1e-9) {
				new_expr->add_operand(std::make_shared<variable>(name));
				continue;
			}

			new_expr->add_operand(std::make_shared<multiplication>(
				std::make_shared<constant>(coeff),
				expr
			));
		}
		return new_expr;
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
			if (i != 0) {
				std::string spaces(print_policies.product.operand_spaces, ' ');
				bool is_val = std::dynamic_pointer_cast<constant>(operands[i]).get();
				if (is_val || print_policies.product.use_stars_for_subexprs)
					s += spaces + "*";
			}
			s += operands[i]->string(this);
		}
		if (par) s += ")";
		return s;
	}

	bool multiplication::is_ground() const {
		return std::ranges::all_of(operands, [](const auto& op) {
			return op->is_ground();
		});
	}

	ptr<detail::n_operation> multiplication::sorted() const {
		// First sort itself, then sort its subexpressions
		ptr<multiplication> cpy = std::make_shared<multiplication>(*this);

		for (auto& op : cpy->operands) {
			if (auto operation = std::dynamic_pointer_cast<n_operation>(op)) {
				op = operation->sorted();
			}
		}
		std::ranges::sort(cpy->operands, [](const auto& op1, const auto& op2) {
			/* First sort value by type :
			 * 1. Values
			 * 2. Variables
			 * 3. Other subexprs
			 *
			 * If they have the same type, sort by power
			 * If they have the same power or their exponent is not a ground expression, sort by length
			 * If they have the same length, sort alphabetically
			 */
			bool gnd1 = op1->is_ground();
			bool gnd2 = op2->is_ground();
			if (gnd1 != gnd2) {
				return gnd2; // All ground values should be to the right
			}

			double pw1 = 1.0;
			double pw2 = 1.0;
			if (op1->kind() == kind_::power) {
				auto exp = std::dynamic_pointer_cast<power>(op1)->exponent();
				if (exp->is_ground()) pw1 = exp->eval(nullptr);
				else pw1 = std::numeric_limits<double>::max();
			}
			if (op2->kind() == kind_::power) {
				auto exp = std::dynamic_pointer_cast<power>(op2)->exponent();
				if (exp->is_ground()) pw2 = exp->eval(nullptr);
				else pw2 = std::numeric_limits<double>::max();
			}

			if (pw1 != pw2) return pw1 > pw2;

			size_t l1 = op1->string(nullptr).size();
			size_t l2 = op2->string(nullptr).size();
			if (l1 != l2) return l1 < l2;

			return op1->string(nullptr) < op2->string(nullptr);
		});

		return cpy;
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

	/*detail::AdditionNodePtr multiplication::unroll() const {

	}*/

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

	ptr<detail::n_operation> division::sorted() const {
		ptr<division> cpy = std::make_shared<division>(*this);
		std::ranges::sort(cpy->operands, [](const auto& op1, const auto& op2) {
			return op1->string(nullptr) > op2->string(nullptr);
		});
		return cpy;
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

	double power::eval(const detail::Context* ctx) const {
		return std::pow(m_base->eval(ctx), m_exp->eval(ctx));
	}

	std::string power::string(const node* parent) const {
		std::string s;
		std::string sp_b(print_policies.power.operand_spaces_before, ' ');
		std::string sp_a(print_policies.power.operand_spaces_after, ' ');

		if (parent && parent->priority() > priority())
			return "(" + m_base->string(this) + sp_b + "^" + sp_a + m_exp->string(this) + ")";

		return m_base->string(this) + sp_b + "^" + sp_a + m_exp->string(this);
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
