#include "symaths/detail/nodes.h"

#include "symaths/expressions_manip.hpp"
#include "symaths/symaths.hpp"

#include <algorithm>
#include <format>
#include <map>

using namespace sym;


unsigned int detail::node::priority() const {
	return std::visit([](const auto& x) {
		return x.priority;
	}, p_data);
}

double detail::node::eval(const Context* ctx) const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, constant>) {
			return x.value;
		}

		else if constexpr (std::is_same_v<T, symbol>) {
			return ctx->at(x.name);
		}

		else if constexpr (std::is_same_v<T, negation>) {
			return -x.child->eval(ctx);
		}

		else if constexpr (std::is_same_v<T, addition>) {
			double sum = 0.0;
			for (auto& op : x.operands) {
				sum += op->eval(ctx);
			}
			return sum;
		}

		else if constexpr (std::is_same_v<T, multiplication>) {
			double m = 1.0;
			for (auto& op : x.operands) {
				m *= op->eval(ctx);
			}
			return m;
		}

		else if constexpr(std::is_same_v<T, power>) {
			return std::pow(x.base->eval(ctx), x.exponent->eval(ctx));
		}

		return 0.0;
	}, p_data);
}

std::string detail::node::string(const node* parent, bool first) const {
	return std::visit([&](const auto& x) -> std::string {
		using T = std::decay_t<decltype(x)>;

		if (!current_context) {
			throw std::runtime_error("node::string(): current context is null");
		}
		auto& print_policies = current_context->print_policies();

		if constexpr (std::is_same_v<T, constant>) {
			if (parent && std::holds_alternative<addition>(parent->p_data)) {
				return std::format("{}", std::abs(x.value));
			}
			if (!first && x.value < 0 && parent && parent->priority() >= multiplication::priority) {
				return std::format("({})", x.value);
			}
			return std::format("{}", x.value);
		}

		else if constexpr (std::is_same_v<T, symbol>) {
			return x.name;
		}

		else if constexpr (std::is_same_v<T, negation>) {
			// If parent is sum then no need to add "-" sign
			if (parent && std::holds_alternative<addition>(parent->p_data)) {
				return x.child->string(this, first);
			}

			if (first && !(parent && parent->priority() > priority())) {
				return "-" + x.child->string(this, first);
			}

			return "(-" + x.child->string(this,first) + ")";
		}

		else if constexpr (std::is_same_v<T, addition>) {
			bool par = parent && parent->priority() > priority();
			std::string s;
			if (par) s += "(";
			for (int i = 0; i < x.operands.size(); ++i) {
				auto& op = x.operands[i];
				if (std::holds_alternative<negation>(op->p_data) || (std::holds_alternative<constant>(op->p_data) && op->eval(nullptr) < 0)) {
					if (i == 0) {
						s += "-" + op->string(this, true);
					}
					else {
						s += "-" + std::string(print_policies.sum.operand_spaces, ' ') + op->string(this, false);
					}
				}
				else {
					if (i != 0) {
						s += "+" + std::string(print_policies.sum.operand_spaces, ' ');
					}
					s += op->string(this, i == 0);
				}
			}
			if (par) s += ")";
			return s;
		}

		else if constexpr (std::is_same_v<T, multiplication>) {
			bool par = parent && parent->priority() > priority();
			std::string s;
			if (par) s += "(";
			for (int i = 0; i < x.operands.size(); ++i) {
				auto& op = x.operands[i];
				if (i != 0) {
					bool is_val = std::holds_alternative<constant>(op->p_data);
					if (is_val || print_policies.product.use_stars_for_subexprs) {
						s += "*" + std::string(print_policies.product.operand_spaces, ' ');
					}
					else {
						s += std::string(print_policies.product.operand_spaces, ' ');
					}
				}
				s += op->string(this, i == 0);
			}
			if (par) s += ")";
			return s;
		}

		else if constexpr (std::is_same_v<T, power>) {
			bool par = parent && parent->priority() > priority();
			if (par) {
				return "(" + x.base->string(this, false) + "^" + x.exponent->string(this, false) + ")";
			}

			return x.base->string(this, false) + "^" + x.exponent->string(this, false);
		}

		return "";
	}, p_data);
}

bool detail::node::is_ground() const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, constant>) {
			return true;
		}

		if constexpr (std::is_same_v<T, symbol>) {
			return false;
		}

		if constexpr (std::is_same_v<T, negation>) {
			return x.child->is_ground();
		}

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
			return std::ranges::all_of(x.operands, [](const auto& op) { return op->is_ground(); });
		}

		if constexpr (std::is_same_v<T, power>) {
			return x.base->is_ground() && x.exponent->is_ground();
		}

		return false;
	}, p_data);
}


std::size_t node_hash::operator()(const node_key& k) const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		size_t h = typeid(T).hash_code();

		if constexpr (std::is_same_v<T, detail::symbol>)
			h ^= std::hash<std::string>{}(x.name);

		else if constexpr (std::is_same_v<T, detail::constant>)
			h ^= std::hash<double>{}(x.value);

		else if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication>) {
			for (auto* c : x.operands)
				h ^= std::hash<const detail::node*>{}(c) + 0x9e3779b9;
		}

		else if constexpr (std::is_same_v<T, detail::power>) {
			h ^= std::hash<const detail::node*>{}(x.base);
			h ^= std::hash<const detail::node*>{}(x.exponent);
		}

		return h;
	}, k.data);
}

const detail::node* detail::negation::sorted() const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, negation>) {
			return current_context->node_manager().make_negation(x.sorted());
		}
		return current_context->node_manager().make_negation(child);
	}, child->p_data);
}

const detail::node* detail::negation::reduced() const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
			return current_context->node_manager().make_negation(x.reduced());
		}

		// TODO: eliminate a maximum of "-" signs in the expression (--x becomes x, ---x becomes -x, etc...)

		return current_context->node_manager().make_negation(child);
	}, child->p_data);
}

const detail::node* detail::negation::expanded() const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
			return current_context->node_manager().make_negation(x.expanded());
		}
		return current_context->node_manager().make_negation(child);
	}, child->p_data);
}

double get_biggest_power(const detail::node* node) {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		double max = 1.0;
		if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication>) {
			for (auto& op : x.operands) {
				max = std::max(max, get_biggest_power(op));
			}
		}

		// Don't care if it's positive or negative, only exponent is important
		if constexpr (std::is_same_v<T, detail::negation>) {
			return get_biggest_power(x.child);
		}

		if constexpr (std::is_same_v<T, detail::power>) {
			if (x.exponent->is_ground())
				max = x.exponent->eval(nullptr);
			else
				max = std::numeric_limits<double>::max();
		}
		return max;
	}, node->p_data);
}

const detail::node* detail::addition::sorted() const {
	// HOW TO PROCEED : First sort its subexpressions, then sort itself
	std::vector sorted_ops{operands};

	for (auto& op : sorted_ops) {
		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;

			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
				op = x.sorted();
			}
		}, op->p_data);
	}
	std::ranges::sort(sorted_ops, [](const auto& op1, const auto& op2) {
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

		term term1 = extract_term(op1);
		term term2 = extract_term(op2);
		std::string repr1, repr2;
		if (term1.symbolic)
			repr1 = extract_term(op1).symbolic->string(nullptr);
		if (term2.symbolic)
			repr2 = extract_term(op2).symbolic->string(nullptr);

		size_t l1 = repr1.size();
		size_t l2 = repr2.size();
		if (l1 != l2) return l1 < l2;

		return repr1 < repr2;
	});

	return current_context->node_manager().make_add(sorted_ops);
}

const detail::node* detail::addition::reduced() const {
	// In case of other sub-expressions, recursively simplify them
	// Hold each variable and its total coefficient
	std::map<std::string, std::pair<double, const node*>> m;

	// Get all variables' coefficient
	for (auto op : operands) {
		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
				op = x.reduced();
			}
		}, op->p_data);

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
			term term_ = extract_term(op);

			// Don't add term if coefficient is (almost) null
			if (std::abs(term_.coefficient) > 1e-9) {
				std::string name = std::visit([&](auto& x) {
					using T = std::decay_t<decltype(x)>;
					if constexpr (std::is_same_v<T, negation>) {
						return expression(x.child).string();
					}
					return expression(term_.symbolic).string();
				}, term_.symbolic->p_data);
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

	auto& nm = current_context->node_manager();

	// ptr<addition> new_expr = std::make_shared<addition>();
	std::vector<const node*> new_expr;
	for (auto& [name, val] : m) {
		auto [coeff, expr] = val;

		// Don't make a multiplication if it is just a constant
		if (name.empty()) {
			new_expr.push_back(nm.make_constant(coeff));
			continue;
		}

		// If it is (almost) equal to 1, don't create a multiplication
		if (std::abs(coeff - 1.0) < 1e-9) {
			new_expr.push_back(reduce(expr).root);
			continue;
		}

		// If it is (almost) equal to 0, do not create an operand
		if (std::abs(coeff) < 1e-9) {
			continue;
		}

		new_expr.push_back(nm.make_mul({
			nm.make_constant(coeff),
			reduce(expr).root
		}));
	}

	if (new_expr.empty()) {
		return nm.make_constant(0);
	}
	if (new_expr.size() == 1) {
		if (new_expr.front()->is_ground()) {
			return current_context->node_manager().make_constant(new_expr.front()->eval(nullptr));
		}

		return new_expr.front();
	}

	return nm.make_add(new_expr);
}

const detail::node* detail::addition::expanded() const {
	// Just expand sub-expressions, since expanding an addition makes no sense
	std::vector<const node*> new_expr;
	for (auto op : operands) {
		new_expr.push_back(std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;

			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
				return x.expanded();
			}
			return op;
		}, op->p_data));
	}

	if (new_expr.size() == 1) {
		return new_expr.front();
	}
	return current_context->node_manager().make_add(new_expr);
}

const detail::node* detail::multiplication::sorted() const {
	// HOW TO PROCEED : First sort its subexpressions, then sort itself
	std::vector sorted_ops{operands};

	for (auto& op : sorted_ops) {
		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;

			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
				op = x.sorted();
			}
		}, op->p_data);
	}
	std::ranges::sort(sorted_ops, [](const auto& op1, const auto& op2) {
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
		if (std::holds_alternative<power>(op1->p_data)) {
			auto exp = std::get<power>(op1->p_data).exponent;
			if (exp->is_ground()) pw1 = exp->eval(nullptr);
			else pw1 = std::numeric_limits<double>::max();
		}
		if (std::holds_alternative<power>(op2->p_data)) {
			auto exp = std::get<power>(op2->p_data).exponent;
			if (exp->is_ground()) pw2 = exp->eval(nullptr);
			else pw2 = std::numeric_limits<double>::max();
		}

		if (pw1 != pw2) return pw1 > pw2;

		size_t l1 = op1->string(nullptr).size();
		size_t l2 = op2->string(nullptr).size();
		if (l1 != l2) return l1 < l2;

		return op1->string(nullptr) < op2->string(nullptr);
	});

	return current_context->node_manager().make_mul(sorted_ops);
}

struct product_term {
	expression base = 1;
	expression exp = 1;
	bool negated = false;
};

// Internal helper function
// Note that in case of negative symbols :
// -x^2 --> base : -x | exp : 2
product_term extract_products(const detail::node* op) {
	return std::visit([&](auto& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr (std::is_same_v<T, detail::power>) {
			return product_term{x.base, x.exponent};
		}
		// If it's a negation, extract the child's exponent then negate the base
		if constexpr (std::is_same_v<T, detail::negation>) {
			auto term = extract_products(x.child);
			term.negated = true;
			return term;
		}
		return product_term{op, 1};
	}, op->p_data);
}

const detail::node* detail::multiplication::reduced() const {
	// Same function as addition::reduced
	std::map<std::string, product_term> coefficients;
	double global_coeff = 1.0;

	for (auto op : operands) {
		// First reduce sub expressions
		op = std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> ||std::is_same_v<T, negation>) {
				return x.reduced();
			}
			return op;
		}, op->p_data);

		if (op->is_ground()) {
			if (std::abs(op->eval(nullptr)) < 1e-10) {
				return current_context->node_manager().make_constant(0);
			}
			global_coeff *= op->eval(nullptr);
		}
		else {
			product_term result = extract_products(op);
			std::string name = result.base.string();
			if (result.negated) {
				global_coeff *= -1;
			}
			if (coefficients.contains(name)) {
				// a^b * a^c = a^(b+c)
				coefficients[name].exp = coefficients[name].exp + result.exp;
			}
			else {
				coefficients[name] = result;
			}
		}
	}

	// Now, create a new sorted branch

	std::vector<const node*> new_operands;
	if (std::abs(global_coeff - 1.0) > 1e-12 && std::abs(global_coeff + 1.0)) {
		new_operands.push_back(current_context->node_manager().make_constant(std::abs(global_coeff)));
	}
	for (auto& [name, val] : coefficients) {
		auto [base, exp, neg] = val;

		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
				exp = x.reduced();
			}
		}, exp.root->p_data);

		// If base is (almost) equal to 1 or exponent is 0, don't create a multiplication
		if (base.is_ground() && std::abs(base() - 1.0) < 1e-9 || exp.is_ground() && std::abs(exp()) < 1e-9) {
			continue;
		}

		// If exponent is (almost) 1, then do not create a power node
		if (exp.is_ground() && std::abs(exp() - 1) < 1e-9) {
			new_operands.push_back(base.root);
			continue;
		}

		new_operands.push_back(current_context->node_manager().make_pow(base.root, exp.root));
	}
	if (new_operands.empty()) {
		return current_context->node_manager().make_constant(1);
	}
	if (new_operands.size() == 1) {
		if (global_coeff < 0) {
			return current_context->node_manager().make_negation(new_operands.front());
		}
		return new_operands.front();
	}
	const node* result = current_context->node_manager().make_mul(new_operands);
	if (global_coeff < 0) {
		return current_context->node_manager().make_negation(result);
	}
	return result;
}

const detail::node* detail::multiplication::expanded() const {
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
	std::vector<std::vector<const node*>> all_operands;

	for (auto& op : operands) {
		const node* expanded = op;
		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;

			// Expand all subexpressions
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
				expanded = x.expanded();
			}
			if (std::holds_alternative<addition>(expanded->p_data)) {
				all_operands.push_back(std::get<addition>(expanded->p_data).operands);
			}
			else {
				all_operands.push_back({expanded});
			}
		}, op->p_data);
	}

	std::vector<std::vector<const node*>> products;
	products.emplace_back();

	for (const auto& group : all_operands) {
		std::vector<std::vector<const node*>> new_products;

		for (const auto& prefix : products) {
			for (const auto& item : group) {
				std::vector<const node*> next = prefix;
				next.push_back(item);
				new_products.push_back(std::move(next));
			}
		}

		products = std::move(new_products);
	}

	auto result = std::make_shared<addition>();
	std::vector<const node*> final_terms;
	for (const auto& prod : products) {
		std::vector<const node*> final_group_terms;
		for (const auto& factor : prod) {
			if (factor && !(factor->is_ground() && std::abs(factor->eval(nullptr) - 1) < 1e-9)) {
				final_group_terms.push_back(factor);
			}
		}
		final_terms.push_back(current_context->node_manager().make_mul(final_group_terms));
	}
	if (final_terms.empty()) {
		return nullptr;
	}
	if (final_terms.size() == 1) {
		return final_terms.front();
	}

	return current_context->node_manager().make_add(final_terms);
}

const detail::node* detail::power::reduced() const {
	const node* base_ = std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
			return x.reduced();
		}
		return base;
	}, base->p_data);

	const node* exp_ = std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, negation>) {
			return x.reduced();
		}
		return exponent;
	}, exponent->p_data);

	return current_context->node_manager().make_pow(base_, exp_);
}

const detail::node* detail::power::expanded() const {
	return current_context->node_manager().make_pow(base, exponent);
	/*return std::visit([&](auto& x) {
		using T = std::decay_t<decltype(x)>;

		// If the base is a multiplication, it's easy : distribute power to each operand
		if constexpr (std::is_same_v<T, multiplication>) {
			std::vector<const node*> result;

		}
	}, base->p_data);*/
}


const detail::node* node_manager_t::intern(detail::node::internal_data_t data) {
	node_key key{data};

	auto it = table.find(key);
	if (it != table.end())
		return it->second;

	auto n = std::make_unique<detail::node>();
	n->p_hash = node_hash{}(key);
	n->p_data = std::move(data);

	arena.push_back(std::move(n));
	table.emplace(std::move(key), arena.back().get());
	return arena.back().get();
}

template<typename T>
std::vector<const detail::node*> flatten(const std::vector<const detail::node*>& args) {
	std::vector<const detail::node*> flat;
	for (auto* a : args) {
		if (std::holds_alternative<T>(a->p_data)) {
			auto& operands = std::get<T>(a->p_data).operands;
			flatten<T>(operands);
			flat.append_range(operands);
		}
		else {
			flat.push_back(a);
		}
	}
	return flat;
}

detail::node::internal_data_t reduce_negations(detail::negation arg) {
	// 2 negations = +
	if (std::holds_alternative<detail::negation>(arg.child->p_data)) {
		return std::get<detail::negation>(arg.child->p_data).child->p_data;
	}
	return arg;
}

const detail::node* node_manager_t::make_symbol(const std::string& name) {
	return intern(detail::symbol{name});
}

const detail::node* node_manager_t::make_constant(double v) {
	return intern(detail::constant{v});
}

const detail::node* node_manager_t::make_negation(const detail::node* node) {
	return intern(reduce_negations(detail::negation{node}));
}

const detail::node* node_manager_t::make_add(const std::vector<const detail::node*>& operands) {
	return intern(detail::addition{flatten<detail::addition>(operands)});
}

const detail::node* node_manager_t::make_mul(const std::vector<const detail::node*>& operands) {
	return intern(detail::multiplication{flatten<detail::multiplication>(operands)});
}

const detail::node* node_manager_t::make_div(const detail::node* a, const detail::node* b) {
	return make_mul({a, make_pow(b, make_constant(-1))});
}

const detail::node* node_manager_t::make_pow(const detail::node* b, const detail::node* e) {
	return intern(detail::power{b, e});
}
