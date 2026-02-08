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
		if (value < 0 && parent && parent->kind() != kind_::negate)
			return std::format("({})", value);
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
			if (i != 0 && operands[i]->kind() != kind_::negate) {
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

	double get_biggest_power(detail::NodePtr node) {
		double max = 1.0;

		if (auto n_op = std::dynamic_pointer_cast<detail::n_operation>(node)) {
			for (auto& subop : n_op->get_operands()) {
				max = std::max(max, get_biggest_power(subop));
			}
		}

		if (auto pow = std::dynamic_pointer_cast<power>(node)) {
			if (pow->exponent()->is_ground())
				max = pow->exponent()->eval(nullptr);
			else
				max = std::numeric_limits<double>::max();
		}

		return max;
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

			double pw1 = get_biggest_power(op1);
			double pw2 = get_biggest_power(op2);

			if (pw1 != pw2) return pw1 > pw2;

			detail::term term1 = detail::extract_term(op1);
			detail::term term2 = detail::extract_term(op2);
			std::string repr1, repr2;
			if (term1.symbolic)
				repr1 = detail::extract_term(op1).symbolic->string(nullptr);
			if (term2.symbolic)
				repr2 = detail::extract_term(op2).symbolic->string(nullptr);

			size_t l1 = repr1.size();
			size_t l2 = repr2.size();
			if (l1 != l2) return l1 < l2;

			return repr1 < repr2;
		});

		return cpy;
	}

	void addition::flatten() {
		std::vector<std::pair<std::vector<detail::NodePtr>::iterator, ptr<addition>>> to_replace; // Holds iterator to replace
		for (auto it = operands.begin(); it != operands.end(); ++it) {
			auto& op = *it;
			if (auto n_op = std::dynamic_pointer_cast<n_operation>(op)) {
				n_op->flatten();
				if (n_op->kind() == kind_::addition) {
					to_replace.emplace_back(it, std::reinterpret_pointer_cast<addition>(n_op)); // Deleting or adding now invalidates vector iterators
				}
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
		for (auto op : operands) {
			if (auto n_op = std::dynamic_pointer_cast<n_operation>(op)) {
				op = n_op->reduced();
			}

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
			auto [coeff, expr] = val;

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

			// If it is (almost) equal to 0, do not create an operand
			if (std::abs(coeff) < 1e-9) {
				continue;
			}

			new_expr->add_operand(std::make_shared<multiplication>(
				std::make_shared<constant>(coeff),
				expr
			));
		}
		new_expr->flatten();
		return new_expr;
	}

	double negate::eval(const detail::Context* ctx) const {
		return -child->eval(ctx);
	}

	std::string negate::string(const node* parent) const {
		if (parent && parent->kind() == kind_::addition) {
			std::string spaces(print_policies.sum.operand_spaces, ' ');
			return spaces + "-" + spaces + child->string(this);
		}

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
				return gnd1; // All ground values should be to the left
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

	// Internal function / takes an operation, and develop all of its operands
	void cartesian_product(ptr<detail::n_operation>& expr) {
		const auto& operands = expr->get_operands();

		for (int i = static_cast<int>(operands.size()) - 2; i >= 0; ++i) {
			auto op1 = operands[i];
			auto op2 = operands[i + 1];
			if (auto suboperation = std::dynamic_pointer_cast<detail::n_operation>(op1)) {
				op1 = suboperation->expand();
			}
		}
	}

	ptr<detail::n_operation> multiplication::expand() const {
		// HOW : recursively develop expressions and subexpressions
		//    BASE          DEVELOP       REDUCE
		// (x+3)(x-2) --> x^2-2x+3x-6 --> x^2+x-6
		//       BASE                 DEVELOP              REDUCE                   DEVELOP                   REDUCE
		// (x+3)(x-2)(2x+6) --> (x^2-2x+3x-6)(2x+6) --> (x^2+x-6)(2x+6) --> 2x^3+6x^2+2x^2+6x+12x+36 --> 2x^3+8x^2+18x+36
		//
		// Algorithm :
		// Take first two operands of the multiplication
		// Create an addition node
		// For each subexpressions in first operand
		//     For each subexpressions in second operand
		std::vector<std::vector<detail::NodePtr>> all_operands;

		for (auto& op : operands) {
			detail::NodePtr expanded;
			if (auto ex = std::dynamic_pointer_cast<n_operation>(op))
				expanded = ex->expand();
			else
				expanded = op;

			if (auto add = std::dynamic_pointer_cast<addition>(expanded)) {
				all_operands.push_back(add->get_operands());
			} else {
				all_operands.push_back({ expanded });
			}
		}

		std::vector<std::vector<detail::NodePtr>> products;
		products.emplace_back();

		for (const auto& group : all_operands) {
			std::vector<std::vector<detail::NodePtr>> new_products;

			for (const auto& prefix : products) {
				for (const auto& item : group) {
					std::vector<detail::NodePtr> next = prefix;
					next.push_back(item);
					new_products.push_back(std::move(next));
				}
			}

			products = std::move(new_products);
		}

		auto result = std::make_shared<addition>();
		for (const auto& prod : products) {
			auto mul = std::make_shared<multiplication>();
			for (const auto& factor : prod) {
				if (factor && !(factor->is_ground() && std::abs(factor->eval(nullptr)) < 1e-9)) {
					mul->add_operand(factor);
				}
			}
			result->add_operand(mul);
		}

		result->flatten();
		if (result->get_operands().size() <= 1) {
			return std::reinterpret_pointer_cast<n_operation>(result->get_operands()[0]);
		}
		return result;
	}

	struct product_term {
		expression base = 1;
		expression exp = 1;
	};

	// Internal helper function
	product_term extract_products(detail::NodePtr op) {
		if (auto pow = std::dynamic_pointer_cast<power>(op)) {
			return {pow->base(), pow->exponent()};
		}

		return {op, 1};
	}

	detail::NodePtr multiplication::reduced() {
		// Same function as addition::reduced
		std::map<std::string, product_term> coefficients;

		for (auto& op : operands) {
			if (auto n_op = std::dynamic_pointer_cast<n_operation>(op)) {
				op = n_op->reduced();
			}

			if (op->is_ground()) {
				if (std::abs(op->eval(nullptr)) < 1e-9) {
					return std::make_shared<constant>(0);
				}
				if (coefficients.contains("")) {
					coefficients[""].base = coefficients[""].base * op->eval(nullptr);
				}
				else {
					coefficients[""] = {op->eval(nullptr), 1};
				}
			}
			else {
				product_term result = extract_products(op);
				std::string name = result.base.string();
				if (coefficients.contains(name)) {
					coefficients[name].exp = coefficients[name].exp + result.exp;
				}
				else {
					coefficients[name] = result;
				}
			}
		}

		// Now, create a new sorted branch

		if (coefficients.size() <= 1) {
			auto [base, exp] = coefficients.begin()->second;
			if (auto opexp = std::dynamic_pointer_cast<n_operation>(exp.root)) {
				exp = opexp->reduced();
			}
			if (exp.is_ground() && std::abs(exp() - 1) < 1e-9) {
				return base.root;
			}
			return std::make_shared<power>(base.root, exp.root);
		}

		ptr<multiplication> new_expr = std::make_shared<multiplication>();
		for (auto& [name, val] : coefficients) {
			auto [base, exp] = val;

			if (auto opexp = std::dynamic_pointer_cast<n_operation>(exp.root)) {
				exp = opexp->reduced();
			}

			// If base is (almost) equal to 1 or exponent is 0, don't create a multiplication
			if (base.is_ground() && std::abs(base() - 1.0) < 1e-9 || exp.is_ground() && std::abs(exp()) < 1e-9) {
				continue;
			}

			// If exponent is (almost) 1, then do not create a power node
			if (exp.is_ground() && std::abs(exp() - 1) < 1e-9) {
				new_expr->add_operand(base.root);
				continue;
			}

			new_expr->add_operand(std::make_shared<power>(
				base.root,
				exp.root
			));
		}
		new_expr->flatten();
		return new_expr;
	}

	void multiplication::flatten() {
		std::vector<std::pair<int, ptr<multiplication>>> to_replace; // Holds iterator to replace
		for (auto it = operands.begin(); it != operands.end(); ++it) {
			auto& op = *it;
			if (auto n_op = std::dynamic_pointer_cast<n_operation>(op)) {
				n_op->flatten();
				if (n_op->kind() == kind_::multiplication) {
					to_replace.emplace_back(std::distance(operands.begin(), it), std::reinterpret_pointer_cast<multiplication>(n_op)); // Deleting or adding now invalidates vector iterators
				}
			}
		}

		for (auto rit = to_replace.rbegin(); rit != to_replace.rend(); ++rit) {
			auto& [i, op] = *rit;

			// First erase the old operand, and then insert its operands at its position
			operands.erase(operands.begin() + i);
			operands.insert_range(operands.begin() + i, op->get_operands());
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
