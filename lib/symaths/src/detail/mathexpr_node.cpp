#include "symaths/detail/mathexpr_node.hpp"

#include "symaths/builtin_functions.hpp"
#include "symaths/base_functions.hpp"
#include "symaths/context_table.hpp"
#include "symaths/expressions_manip.hpp"
#include "symaths/symaths.hpp"
#include "symaths/detail/node_manager.hpp"
#include "symaths/utils/helpers.hpp"
#include "symaths/utils/maths.hpp"

#include <algorithm>
#include <format>
#include <map>


using namespace sym;

namespace sym::detail {
	void find_all_paths_r(const mathexpr_node* current, const mathexpr_node* target, node_path_t& current_path, std::vector<node_path_t>& paths);
	void find_symbols_r(const mathexpr_node* current, std::vector<const mathexpr_node*>& result);
}

static bool builtin_arg_is_ground(const detail::expression_value_t& arg) {
	if (auto* mp = std::get_if<const detail::mathexpr_node*>(&arg)) return (*mp)->is_ground();
	return true;
}

static bool builtin_arg_depends_on(const detail::expression_value_t& arg, const detail::mathexpr_node* n) {
	if (auto* mp = std::get_if<const detail::mathexpr_node*>(&arg)) return (*mp)->depends_on(n);
	return false;
}

static void builtin_arg_find_all_paths(const detail::expression_value_t& arg, const detail::mathexpr_node* target, detail::node_path_t& path, std::vector<detail::node_path_t>& paths) {
	if (auto* mp = std::get_if<const detail::mathexpr_node*>(&arg)) find_all_paths_r(*mp, target, path, paths);
}

static void builtin_arg_find_symbols(const detail::expression_value_t& arg, std::vector<const detail::mathexpr_node*>& result) {
	if (auto* mp = std::get_if<const detail::mathexpr_node*>(&arg)) find_symbols_r(*mp, result);
}

static std::string builtin_arg_string(const detail::expression_value_t& arg) {
	return std::visit(overloaded {
		[&](const bool& b) -> std::string { return b ? "true" : "false"; },
		[&](const number n) -> std::string { return n.string(); },
		[&](const exception& e) -> std::string { return e.string(); },
		[&](const auto& p) -> std::string { return p->string(); }
	}, arg);
}

void detail::find_all_paths_r(const mathexpr_node* current, const mathexpr_node* target, node_path_t& current_path, std::vector<node_path_t>& paths) {
	if (!current) return;

	current_path.push_back(current);
	if (current == target) {
		paths.push_back(current_path);
	}

	std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, expr_negation>) {
			find_all_paths_r(x.child, target, current_path, paths);
		}

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
			for (auto* c : x.operands) {
				find_all_paths_r(c, target, current_path, paths);
			}
		}

		if constexpr (std::is_same_v<T, power>) {
			find_all_paths_r(x.base, target, current_path, paths);
			find_all_paths_r(x.exponent, target, current_path, paths);
		}

		if constexpr (std::is_same_v<T, mathfunc_call>) {
			for (auto* c : x.args) {
				find_all_paths_r(c, target, current_path, paths);
			}
		}

		if constexpr (std::is_same_v<T, builtin_call>) {
			for (auto& c : x.args) {
				builtin_arg_find_all_paths(c, target, current_path, paths);
			}
		}
	}, current->p_data);
	current_path.pop_back();
}

void detail::find_symbols_r(const mathexpr_node* current, std::vector<const mathexpr_node*>& result) {
	if (!current) return;

	std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, symbol>) {
			if (std::ranges::find(result, current) == result.end()) {
				result.push_back(current);
			}
		}

		if constexpr (std::is_same_v<T, expr_negation>) {
			find_symbols_r(x.child, result);
		}

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
			for (auto* c : x.operands) {
				find_symbols_r(c, result);
			}
		}

		if constexpr (std::is_same_v<T, power>) {
			find_symbols_r(x.base, result);
			find_symbols_r(x.exponent, result);
		}

		if constexpr (std::is_same_v<T, mathfunc_call>) {
			for (auto* c : x.args) {
				find_symbols_r(c, result);
			}
		}

		if constexpr (std::is_same_v<T, builtin_call>) {
			for (auto& c : x.args) {
				builtin_arg_find_symbols(c, result);
			}
		}
	}, current->p_data);
}


unsigned int detail::mathexpr_node::priority() const {
	return std::visit([](const auto& x) {
		return x.priority;
	}, p_data);
}

number detail::mathexpr_node::eval(context_table_t* ctx, std::ostream* out) const {
	number num = std::visit(overloaded {
		[&](const constant& x) -> number { return x.value; },
		[&](const symbol& x) -> number {
			auto n = ctx->find_number(x.name);
			if (!n) {
				exception e{std::format(R"(Reference "{}" isn't of the required "number" type.)", x.name)};
				if (out) *out << e.string() << "\n";
				return numbers::nan{};
			}
			return n.value();
		},
		[&](const expr_negation& x) -> number { return -x.child->eval(ctx); },
		[&](const addition& x) -> number {
			number sum = numbers::natural(0);
			for (auto& op : x.operands) {
				sum += op->eval(ctx);
			}
			return sum;
		},
		[&](const multiplication& x) -> number {
			number prod = numbers::natural(1);
			for (auto& op : x.operands) {
				prod *= op->eval(ctx);
			}
			return prod;
		},
		[&](const power& x) -> number {
			return pow_calc(x.base->eval(ctx), x.exponent->eval(ctx));
		},
		[&](const mathfunc_call& x) -> number {
			const auto& func = get_func(funcs::builtin_fn_id{x.f_id});
			return func.eval(x.args);
		},
		[&](const builtin_call& x) -> number {
			return resolve(out, ctx)->eval(ctx);
		}
	}, p_data);
	num.downcast();
	return num;
}

const detail::mathexpr_node* detail::mathexpr_node::resolve(std::ostream* p_out, context_table_t* ctx) const {
	if (!ctx) ctx = &current_context->context_table();
	auto& nm = current_context->node_manager();
	return std::visit(overloaded {
		[&](const symbol& x) -> const mathexpr_node* {
			auto val = ctx->find(x.name);
			if (!val || !val.value()) {
				return nm.make_symbol(x.name); // Either not in context_table or not initialized.
			}
			if (val.value().type() != mathexpr_) {
				exception e{std::format(R"(Reference "{}" isn't of the required "mathexpr" type.)", x.name)};
				if (p_out) *p_out << e.string() << "\n";
			}
			return val.value().cast<const mathexpr_node*>(); // Get the mathexpr mapped to this name
		},
		[&](const builtin_call& x) -> const mathexpr_node* {
			std::vector<expression_value_t> resolved_args;
			resolved_args.reserve(x.args.size());
			for (auto& arg : x.args) {
				if (auto* mp = std::get_if<const mathexpr_node*>(&arg)) {
					resolved_args.emplace_back((*mp)->resolve(p_out, ctx));
				} else {
					resolved_args.push_back(arg);
				}
			}
			const auto& b = get_builtin(x.id);
			object result = b.handler(resolved_args, ctx);
			if (result.type() == mathexpr_) {
				return result.cast<const mathexpr_node*>();
			}
			return this;
		},
		[&](const expr_negation& x) -> const mathexpr_node* {
			auto* child = x.child->resolve(p_out, ctx);
			if (child == x.child) return this;
			return nm.make_negation(child);
		},
		[&](const addition& x) -> const mathexpr_node* {
			std::vector<const mathexpr_node*> ops;
			bool changed = false;
			for (auto* op : x.operands) {
				auto* r = op->resolve(p_out, ctx);
				ops.push_back(r);
				if (r != op) changed = true;
			}
			if (!changed) return this;
			return nm.make_add(ops);
		},
		[&](const multiplication& x) -> const mathexpr_node* {
			std::vector<const mathexpr_node*> ops;
			bool changed = false;
			for (auto* op : x.operands) {
				auto* r = op->resolve(p_out, ctx);
				ops.push_back(r);
				if (r != op) changed = true;
			}
			if (!changed) return this;
			return nm.make_mul(ops);
		},
		[&](const power& x) -> const mathexpr_node* {
			auto* b = x.base->resolve(p_out, ctx);
			auto* e = x.exponent->resolve(p_out, ctx);
			if (b == x.base && e == x.exponent) return this;
			return nm.make_pow(b, e);
		},
		[&](const mathfunc_call& x) -> const mathexpr_node* {
			std::vector<const mathexpr_node*> resolved_args;
			bool changed = false;
			for (auto* arg : x.args) {
				auto* r = arg->resolve(p_out, ctx);
				resolved_args.push_back(r);
				if (r != arg) changed = true;
			}
			if (!changed) return this;
			return nm.make_func(x.f_id, resolved_args);
		},
		[&](const auto&) -> const mathexpr_node* {
			return this;
		},
	}, p_data);
}

bool should_be_preceeded_by_star(const detail::mathexpr_node* parent, const detail::mathexpr_node* node) {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, detail::constant> || std::is_same_v<T, detail::mathfunc_call> || std::is_same_v<T, detail::builtin_call>) {
			return parent->priority() > node->priority();
		}

		if constexpr (std::is_same_v<T, detail::symbol>) {
			return false;
		}

		if constexpr (std::is_same_v<T, detail::expr_negation>) {
			return parent->priority() > node->priority();
		}

		if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication>) {
			return should_be_preceeded_by_star(parent, x.operands[0]);
		}

		if constexpr (std::is_same_v<T, detail::power>) {
			return should_be_preceeded_by_star(parent, x.base);
		}

		return true;
	}, node->p_data);
}

std::string detail::mathexpr_node::string(const mathexpr_node* parent, bool first) const {
	return std::visit([&](const auto& x) -> std::string {
		using T = std::decay_t<decltype(x)>;

		if (!current_context) {
			throw std::runtime_error("node::string(): current context is null");
		}
		auto& print_policies = current_context->print_policies();

		if constexpr (std::is_same_v<T, constant>) {
			if (parent && std::holds_alternative<addition>(parent->p_data)) {
				return std::format("{}", abs_calc(x.value).string());
			}
			if (!first && x.value.template get<double>() < 0 && parent && parent->priority() >= multiplication::priority) {
				return std::format("({})", x.value.string());
			}
			return std::format("{}", x.value.string());
		}

		else if constexpr (std::is_same_v<T, symbol>) {
			return x.name;
		}

		else if constexpr (std::is_same_v<T, expr_negation>) {
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
				if (std::holds_alternative<expr_negation>(op->p_data) || (std::holds_alternative<constant>(op->p_data) && op->eval(nullptr).template get<double>() < 0)) {
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

		else if constexpr (std::is_same_v<T, mathfunc_call>) {
			const auto& func = get_func(funcs::builtin_fn_id{x.f_id});
			std::string s = std::string(func.name) + "(";
			for (int i = 0; i < x.args.size(); ++i) {
				auto& arg = x.args[i];
				if (i != 0) {
					s += ", ";
				}
				s += arg->string(this, i == 0);
			}
			return s + ")";
		}

		else if constexpr (std::is_same_v<T, builtin_call>) {
			const auto& b = get_builtin(x.id);
			std::string s = std::string(b.name) + "(";
			for (int i = 0; i < x.args.size(); ++i) {
				auto& arg = x.args[i];
				if (i != 0) {
					s += ", ";
				}
				s += builtin_arg_string(arg);
			}
			return s + ")";
		}

		return "";
	}, p_data);
}

bool detail::mathexpr_node::is_ground() const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, constant>) {
			return true;
		}

		if constexpr (std::is_same_v<T, symbol>) {
			return false;
		}

		if constexpr (std::is_same_v<T, expr_negation>) {
			return x.child->is_ground();
		}

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
			return std::ranges::all_of(x.operands, [](const auto& op) { return op->is_ground(); });
		}

		if constexpr (std::is_same_v<T, power>) {
			return x.base->is_ground() && x.exponent->is_ground();
		}

		if constexpr (std::is_same_v<T, mathfunc_call>) {
			return std::ranges::all_of(x.args, [](const auto& arg) { return arg->is_ground(); });
		}

		if constexpr (std::is_same_v<T, builtin_call>) {
			return std::ranges::all_of(x.args, builtin_arg_is_ground);
		}

		return false;
	}, p_data);
}

bool detail::mathexpr_node::depends_on(const mathexpr_node* n) const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr (std::is_same_v<T, constant>) {
			return false;
		}

		if constexpr (std::is_same_v<T, symbol>) {
			return this == n;
		}

		if constexpr (std::is_same_v<T, expr_negation>) {
			return x.child->depends_on(n);
		}

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication>) {
			return std::ranges::any_of(x.operands, [&](const auto& op) { return op->depends_on(n); });
		}

		if constexpr (std::is_same_v<T, power>) {
			return x.base->depends_on(n) || x.exponent->depends_on(n);
		}

		if constexpr (std::is_same_v<T, mathfunc_call>) {
			return std::ranges::any_of(x.args, [&](const auto& arg) { return arg->depends_on(n); });
		}

		if constexpr (std::is_same_v<T, builtin_call>) {
			return std::ranges::any_of(x.args, [&](const auto& a) { return builtin_arg_depends_on(a, n); });
		}

		return false;
	}, p_data);
}

std::vector<detail::node_path_t> detail::search_node(const mathexpr_node* parent, const mathexpr_node* to_search) {
	std::vector<node_path_t> paths;
	node_path_t current_path;
	find_all_paths_r(parent, to_search, current_path, paths);
	return paths;
}

std::vector<const detail::mathexpr_node*> detail::list_symbols(const mathexpr_node* parent) {
	std::vector<const mathexpr_node*> result;
	find_symbols_r(parent, result);
	return result;
}


const detail::mathexpr_node* detail::expr_negation::sorted() const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, expr_negation>) {
			return current_context->node_manager().make_negation(x.sorted());
		}
		return current_context->node_manager().make_negation(child);
	}, child->p_data);
}

const detail::mathexpr_node* detail::expr_negation::reduced() const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
			return current_context->node_manager().make_negation(x.reduced());
		}

		// TODO: eliminate a maximum of "-" signs in the expression (--x becomes x, ---x becomes -x, etc...)

		return current_context->node_manager().make_negation(child);
	}, child->p_data);
}

const detail::mathexpr_node* detail::expr_negation::expanded() const {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
			return current_context->node_manager().make_negation(x.expanded());
		}
		return current_context->node_manager().make_negation(child);
	}, child->p_data);
}

double get_biggest_power(const detail::mathexpr_node* node) {
	return std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		double max = 1.0;
		if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication>) {
			for (auto& op : x.operands) {
				max = std::max(max, get_biggest_power(op));
			}
		}

		// Don't care if it's positive or negative, only exponent is important
		if constexpr (std::is_same_v<T, detail::expr_negation>) {
			return get_biggest_power(x.child);
		}

		if constexpr (std::is_same_v<T, detail::power>) {
			if (x.exponent->is_ground())
				max = x.exponent->eval(nullptr).template get<double>();
			else
				max = std::numeric_limits<double>::max();
		}
		return max;
	}, node->p_data);
}

const detail::mathexpr_node* detail::addition::sorted() const {
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
			repr1 = extract_term(op1).symbolic->string();
		if (term2.symbolic)
			repr2 = extract_term(op2).symbolic->string();

		size_t l1 = repr1.size();
		size_t l2 = repr2.size();
		if (l1 != l2) return l1 < l2;

		return repr1 < repr2;
	});

	return current_context->node_manager().make_add(sorted_ops);
}

const detail::mathexpr_node* detail::addition::reduced() const {
	// In case of other sub-expressions, recursively simplify them
	// Hold each variable and its total coefficient
	std::map<std::string, std::pair<number, const mathexpr_node*>> m;

	// Get all variables' coefficient
	for (auto op : operands) {
		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
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
			if (std::abs(term_.coefficient.get<double>()) > 1e-12) {
				std::string name = std::visit([&](auto& x) {
					using T = std::decay_t<decltype(x)>;
					if constexpr (std::is_same_v<T, expr_negation>) {
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
	std::vector<const mathexpr_node*> new_expr;
	for (auto& [name, val] : m) {
		auto [coeff, expr] = val;

		// If it is (almost) equal to 0, do not create an operand
		if (std::abs(coeff.get<double>()) < 1e-12) {
			continue;
		}

		// Don't make a multiplication if it is just a constant
		if (name.empty()) {
			new_expr.push_back(nm.make_constant(coeff));
			continue;
		}

		// If it is (almost) equal to 1, don't create a multiplication
		if (std::abs(coeff.get<double>() - 1.0) < 1e-12) {
			new_expr.push_back(reduce(expr).root);
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

const detail::mathexpr_node* detail::addition::expanded() const {
	// Just expand sub-expressions, since expanding an addition makes no sense
	std::vector<const mathexpr_node*> new_expr;
	for (auto op : operands) {
		new_expr.push_back(std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;

			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
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

const detail::mathexpr_node* detail::multiplication::sorted() const {
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
			if (exp->is_ground()) pw1 = exp->eval(nullptr).template get<double>();
			else pw1 = std::numeric_limits<double>::max();
		}
		if (std::holds_alternative<power>(op2->p_data)) {
			auto exp = std::get<power>(op2->p_data).exponent;
			if (exp->is_ground()) pw2 = exp->eval(nullptr).template get<double>();
			else pw2 = std::numeric_limits<double>::max();
		}

		if (pw1 != pw2) return pw1 > pw2;

		size_t l1 = op1->string().size();
		size_t l2 = op2->string().size();
		if (l1 != l2) return l1 < l2;

		return op1->string() < op2->string();
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
product_term extract_products(const detail::mathexpr_node* op) {
	return std::visit([&](auto& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr (std::is_same_v<T, detail::power>) {
			return product_term{x.base, x.exponent};
		}
		// If it's a negation, extract the child's exponent then negate the base
		if constexpr (std::is_same_v<T, detail::expr_negation>) {
			auto term = extract_products(x.child);
			term.negated = true;
			return term;
		}
		return product_term{op, 1};
	}, op->p_data);
}

const detail::mathexpr_node* detail::multiplication::reduced() const {
	// Same function as addition::reduced
	std::map<std::string, product_term> coefficients;
	number global_coeff = numbers::natural(1);

	for (auto op : operands) {
		// First reduce sub expressions
		op = std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> ||std::is_same_v<T, expr_negation>) {
				return x.reduced();
			}
			return op;
		}, op->p_data);

		bool is_func = std::holds_alternative<mathfunc_call>(op->p_data);
		if (op->is_ground() && (!is_func || (is_func && !current_context->refactoring_rules().keep_ground_functions))) {
			if (std::abs(op->eval(nullptr).get<double>()) < 1e-12) {
				return current_context->node_manager().make_constant(0);
			}
			global_coeff *= op->eval(nullptr);
		}
		else {
			product_term result = extract_products(op);
			std::string name = result.base.string();
			if (result.negated) {
				global_coeff = -global_coeff;
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

	std::vector<const mathexpr_node*> new_operands;
	if (std::abs(global_coeff.get<double>() - 1.0) > 1e-12 && std::abs(global_coeff.get<double>() + 1.0)) {
		new_operands.push_back(current_context->node_manager().make_constant(std::abs(global_coeff.get<double>())));
	}
	for (auto& [name, val] : coefficients) {
		auto [base, exp, neg] = val;

		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
				exp = x.reduced();
			}
		}, exp.root->p_data);

		// If base is (almost) equal to 1 or exponent is 0, don't create a multiplication
		if (base.is_ground() && std::abs(base().get<double>() - 1.0) < 1e-12 || exp.is_ground() && std::abs(exp().get<double>()) < 1e-12) {
			continue;
		}

		// If exponent is (almost) 1, then do not create a power node
		if (exp.is_ground() && std::abs(exp().get<double>() - 1) < 1e-12) {
			new_operands.push_back(base.root);
			continue;
		}

		new_operands.push_back(current_context->node_manager().make_pow(base.root, exp.root));
	}
	if (new_operands.empty()) {
		return current_context->node_manager().make_constant(1);
	}
	if (new_operands.size() == 1) {
		if (global_coeff.get<double>() < 0) {
			return current_context->node_manager().make_negation(new_operands.front());
		}
		return new_operands.front();
	}
	const mathexpr_node* result = current_context->node_manager().make_mul(new_operands);
	if (global_coeff.get<double>() < 0) {
		return current_context->node_manager().make_negation(result);
	}
	return result;
}

const detail::mathexpr_node* detail::multiplication::expanded() const {
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
	std::vector<std::vector<const mathexpr_node*>> all_operands;

	for (auto& op : operands) {
		const mathexpr_node* expanded = op;
		std::visit([&](auto& x) {
			using T = std::decay_t<decltype(x)>;

			// Expand all subexpressions
			if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
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

	std::vector<std::vector<const mathexpr_node*>> products;
	products.emplace_back();

	for (const auto& group : all_operands) {
		std::vector<std::vector<const mathexpr_node*>> new_products;

		for (const auto& prefix : products) {
			for (const auto& item : group) {
				std::vector<const mathexpr_node*> next = prefix;
				next.push_back(item);
				new_products.push_back(std::move(next));
			}
		}

		products = std::move(new_products);
	}

	auto result = std::make_shared<addition>();
	std::vector<const mathexpr_node*> final_terms;
	for (const auto& prod : products) {
		std::vector<const mathexpr_node*> final_group_terms;
		for (const auto& factor : prod) {
			if (factor && !(factor->is_ground() && std::abs(factor->eval(nullptr).get<double>() - 1) < 1e-12)) {
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

const detail::mathexpr_node* detail::power::reduced() const {
	const mathexpr_node* base_ = std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
			return x.reduced();
		}
		return base;
	}, base->p_data);

	const mathexpr_node* exp_ = std::visit([&](const auto& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
			return x.reduced();
		}
		return exponent;
	}, exponent->p_data);

	return current_context->node_manager().make_pow(base_, exp_);
}

const detail::mathexpr_node* detail::power::expanded() const {
	return std::visit([&](auto& x) {
		using T = std::decay_t<decltype(x)>;

		const mathexpr_node* expanded_base = base;
		if constexpr (std::is_same_v<T, addition> || std::is_same_v<T, multiplication> || std::is_same_v<T, power> || std::is_same_v<T, expr_negation>) {
			expanded_base = x.expanded();
		}

		// If the base is a multiplication, it's easy : distribute power to each operand
		if (std::holds_alternative<multiplication>(expanded_base->p_data)) {
			multiplication expanded_data = std::get<multiplication>(expanded_base->p_data);
			std::vector<const mathexpr_node*> final_terms;
			for (const auto& op : expanded_data.operands) {
				final_terms.push_back(current_context->node_manager().make_pow(op, exponent));
			}
			return current_context->node_manager().make_mul(final_terms);
		}

		// If the base is an addition, use the multinomial theorem
		if (std::holds_alternative<addition>(expanded_base->p_data)) {
			addition expanded_data = std::get<addition>(expanded_base->p_data);
			auto nd = exponent->eval(nullptr).get<double>();
			auto n = static_cast<unsigned long long>(nd);
			unsigned long long m = expanded_data.operands.size();

			unsigned long long terms_amount = utils::newton_binomial_coefficient(m - 1, n + m - 1);
			if (!exponent->is_ground() ||
				terms_amount > current_context->refactoring_rules().max_power_expansion_terms ||
				!utils::is_integer(nd) || nd <= 0) {
				return current_context->node_manager().make_pow(base, exponent);
			}
		}

		return current_context->node_manager().make_pow(base, exponent);
	}, base->p_data);
}
