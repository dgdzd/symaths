#include "symaths/detail/node_manager.hpp"

#include "symaths/base_functions.hpp"

using namespace sym;

std::size_t expr_node_hash::operator()(const node_key<detail::mathexpr_node>& k) const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		size_t h = typeid(T).hash_code();

		if constexpr (std::is_same_v<T, detail::symbol>)
			h ^= std::hash<std::string>{}(x.name);

		else if constexpr (std::is_same_v<T, detail::constant>)
			std::visit(overloaded {
				[&](const numbers::natural& n) { h ^= std::hash<unsigned long long>{}(n.val); },
				[&](const numbers::integer& n) { h ^= std::hash<long long>{}(n.val); },
				[&](const numbers::rational& n) {
					h ^= std::hash<long long>{}(n.num);
					h ^= std::hash<long long>{}(n.den);
				},
				[&](const numbers::real& n) { h ^= std::hash<double>{}(n.val); },
				[&](const numbers::complex& n) {
					h ^= std::hash<double>{}(n.val.real());
					h ^= std::hash<double>{}(n.val.imag());
				},
				[&](const numbers::nan& n) { h ^= std::hash<double>{}(std::numeric_limits<double>::signaling_NaN()); },
			}, x.value.p_data);

		else if constexpr (std::is_same_v<T, detail::addition> || std::is_same_v<T, detail::multiplication>) {
			for (auto* c : x.operands)
				h ^= std::hash<const detail::mathexpr_node*>{}(c) + 0x9e3779b9;
		}

		else if constexpr (std::is_same_v<T, detail::power>) {
			h ^= std::hash<const detail::mathexpr_node*>{}(x.base);
			h ^= std::hash<const detail::mathexpr_node*>{}(x.exponent);
		}

		else if constexpr (std::is_same_v<T, detail::builtin_call>) {
			h ^= std::hash<uint32_t>{}(x.id);
			for (auto& arg : x.args) {
				std::visit(overloaded {
					[&](const bool& b) { h ^= std::hash<bool>{}(b); },
					[&](const number& n) { h ^= std::hash<double>{}(n.get<double>()); },
					[&](const std::exception& e) { h ^= std::hash<const char*>{}(e.what()) + 0x9e3779b9; },
					[&](const detail::mathexpr_node* node) { h ^= std::hash<decltype(node)>{}(node) + 0x9e3779b9; },
					[&](const detail::predicate_node* node) { h ^= std::hash<decltype(node)>{}(node) + 0x9e3779b9; },
					[&](const detail::set_node* node) { h ^= std::hash<decltype(node)>{}(node) + 0x9e3779b9; },
				}, arg);
			}
		}

		return h;
	}, k.data);
}

std::size_t pred_node_hash::operator()(const node_key<detail::predicate_node>& k) const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		size_t h = typeid(T).hash_code();

		if constexpr (std::is_same_v<T, detail::equality>) {
			h ^= std::hash<std::string>{}("equal");
		}

		else if constexpr (std::is_same_v<T, detail::inequality>) {
			h ^= std::hash<std::string>{}("inequality");
			h ^= std::hash<detail::inequality::kind>{}(x.type);
		}

		else if constexpr (std::is_same_v<T, detail::congruence>) {
			h ^= std::hash<std::string>{}("congruence");
			h ^= std::hash<const detail::mathexpr_node*>{}(x.mod);
		}

		else if constexpr (std::is_same_v<T, detail::element_inclusion>) {
			h ^= std::hash<const detail::set_node*>{}(x.set);
			h ^= std::hash<const detail::mathexpr_node*>{}(x.element);
		}

		else if constexpr (std::is_same_v<T, detail::set_inclusion>) {
			h ^= std::hash<const detail::set_node*>{}(x.set);
			h ^= std::hash<const detail::set_node*>{}(x.subset);
		}

		else if constexpr (std::is_same_v<T, detail::logical_or>) {
			h ^= std::hash<std::string>{}("logic_or");
			h ^= std::hash<const detail::predicate_node*>{}(x.p);
			h ^= std::hash<const detail::predicate_node*>{}(x.q);
		}

		else if constexpr (std::is_same_v<T, detail::logical_and>) {
			h ^= std::hash<std::string>{}("logic_and");
			h ^= std::hash<const detail::predicate_node*>{}(x.p);
			h ^= std::hash<const detail::predicate_node*>{}(x.q);
		}

		if constexpr (std::is_same_v<T, detail::equality> || std::is_same_v<T, detail::inequality> || std::is_same_v<T, detail::congruence>) {
			for (auto* n : x.expressions)
				h ^= std::hash<const detail::mathexpr_node*>{}(n) + 0x9e3779b9;
		}

		return h;
	}, k.data);
}

std::size_t set_node_hash::operator()(const node_key<detail::set_node>& k) const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		size_t h = typeid(T).hash_code();

		// TODO : do things

		return h;
	}, k.data);
}

std::size_t statement_node_hash::operator()(const node_key<detail::statement_node>& k) const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		size_t h = typeid(T).hash_code();

		if constexpr (std::is_same_v<T, detail::stmt_sequence>) {
			for (auto* s : x.statements)
				h ^= std::hash<const detail::statement_node*>{}(s) + 0x9e3779b9;
		}

		else if constexpr (std::is_same_v<T, detail::assignment>) {
			h ^= std::hash<std::string>{}(x.varname);
			h ^= std::hash<const detail::statement_node*>{}(x.value);
			h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(x.expected_type));
		}

		else if constexpr (std::is_same_v<T, detail::expression_statement>) {
			std::visit(overloaded {
				[&](const bool& b) { h ^= std::hash<bool>{}(b); },
				[&](const number& n) { h ^= std::hash<double>{}(n.get<double>()); },
				[&](const exception& e) { h ^= std::hash<const char*>{}(e.what()); },
				[&](const auto& v) { h ^= std::hash<std::decay_t<decltype(v)>>{}(v); }
			}, x.expr);
		}

		else if constexpr (std::is_same_v<T, detail::conditional>) {
			h ^= std::hash<const detail::statement_node*>{}(x.condition);
			h ^= std::hash<const detail::statement_node*>{}(x.then_branch);
			h ^= std::hash<const detail::statement_node*>{}(x.else_branch);
		}

		/*else if constexpr (std::is_same_v<T, detail::while_loop>) {
			h ^= std::hash<const detail::predicate_node*>{}(x.condition);
			h ^= std::hash<const detail::statement_node*>{}(x.body);
		}

		else if constexpr (std::is_same_v<T, detail::for_loop>) {
			h ^= std::hash<const detail::mathexpr_node*>{}(x.variable);
			h ^= std::hash<const detail::set_node*>{}(x.iterable);
			h ^= std::hash<const detail::statement_node*>{}(x.body);
		}*/

		else if constexpr (std::is_same_v<T, detail::function_definition>) {
			h ^= std::hash<std::string>{}(x.name);
			for (auto& p : x.params)
				h ^= std::hash<std::string>{}(p) + 0x9e3779b9;
			h ^= std::hash<const detail::statement_node*>{}(x.body);
		}

		else if constexpr (std::is_same_v<T, detail::function_call>) {
			h ^= std::hash<uint32_t>{}(x.id);
		}

		return h;
	}, k.data);
}


const detail::mathexpr_node* node_manager_t::intern(detail::mathexpr_node::internal_data_t data) {
	node_key<detail::mathexpr_node> key{data};

	auto it = expr_table.find(key);
	if (it != expr_table.end())
		return it->second;

	auto n = std::make_unique<detail::mathexpr_node>();
	n->p_hash = expr_node_hash{}(key);
	n->p_data = std::move(data);

	expr_arena.push_back(std::move(n));
	expr_table.emplace(std::move(key), expr_arena.back().get());
	return expr_arena.back().get();
}

const detail::predicate_node* node_manager_t::intern(detail::predicate_node::internal_data_t data) {
	node_key<detail::predicate_node> key{data};

	auto it = pred_table.find(key);
	if (it != pred_table.end())
		return it->second;

	auto n = std::make_unique<detail::predicate_node>();
	n->p_hash = pred_node_hash{}(key);
	n->p_data = std::move(data);

	pred_arena.push_back(std::move(n));
	pred_table.emplace(std::move(key), pred_arena.back().get());
	return pred_arena.back().get();
}

const detail::set_node* node_manager_t::intern(detail::set_node::internal_data_t data) {
	node_key<detail::set_node> key{data};

	auto it = set_table.find(key);
	if (it != set_table.end())
		return it->second;

	auto n = std::make_unique<detail::set_node>();
	n->p_hash = set_node_hash{}(key);
	n->p_data = std::move(data);

	set_arena.push_back(std::move(n));
	set_table.emplace(std::move(key), set_arena.back().get());
	return set_arena.back().get();
}

const detail::statement_node* node_manager_t::intern(detail::statement_node::internal_data_t data) {
	node_key<detail::statement_node> key{data};

	auto it = stmt_table.find(key);
	if (it != stmt_table.end())
		return it->second;

	auto n = std::make_unique<detail::statement_node>();
	n->p_hash = statement_node_hash{}(key);
	n->p_data = std::move(data);

	stmt_arena.push_back(std::move(n));
	stmt_table.emplace(std::move(key), stmt_arena.back().get());
	return stmt_arena.back().get();
}


template<typename T>
std::vector<const detail::mathexpr_node*> flatten(const std::vector<const detail::mathexpr_node*>& args) {
	std::vector<const detail::mathexpr_node*> flat;
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

detail::mathexpr_node::internal_data_t reduce_negations(detail::expr_negation arg) {
	// 2 negations = +
	if (std::holds_alternative<detail::expr_negation>(arg.child->p_data)) {
		return std::get<detail::expr_negation>(arg.child->p_data).child->p_data;
	}
	return arg;
}

const detail::mathexpr_node* node_manager_t::make_symbol(const std::string& name) {
	return intern(detail::symbol{name});
}

const detail::mathexpr_node* node_manager_t::make_constant(const number& v) {
	return intern(detail::constant{v});
}

const detail::mathexpr_node* node_manager_t::make_constant(double v) {
	return intern(detail::constant{numbers::real{v}});
}

const detail::mathexpr_node* node_manager_t::make_negation(const detail::mathexpr_node* node) {
	return intern(reduce_negations(detail::expr_negation{node}));
}

const detail::mathexpr_node* node_manager_t::make_add(const std::vector<const detail::mathexpr_node*>& operands) {
	return intern(detail::addition{flatten<detail::addition>(operands)});
}

const detail::mathexpr_node* node_manager_t::make_mul(const std::vector<const detail::mathexpr_node*>& operands) {
	return intern(detail::multiplication{flatten<detail::multiplication>(operands)});
}

const detail::mathexpr_node* node_manager_t::make_div(const detail::mathexpr_node* a, const detail::mathexpr_node* b) {
	return make_mul({a, make_pow(b, make_constant(-1))});
}

const detail::mathexpr_node* node_manager_t::make_pow(const detail::mathexpr_node* b, const detail::mathexpr_node* e) {
	return intern(detail::power{b, e});
}

const detail::mathexpr_node* node_manager_t::make_func(uint32_t f_id, const std::vector<const detail::mathexpr_node*>& args) {
	return intern(detail::mathfunc_call{f_id, args});
}

const detail::mathexpr_node* node_manager_t::make_func(funcs::builtin_fn_id f_id, const std::vector<const detail::mathexpr_node*>& args) {
	return intern(detail::mathfunc_call{static_cast<uint32_t>(f_id), args});
}

const detail::mathexpr_node* node_manager_t::make_builtin_call(uint32_t id, const std::vector<detail::expression_value_t>& args) {
	return intern(detail::builtin_call{id, args});
}

const detail::predicate_node* node_manager_t::make_equal(const std::vector<const detail::mathexpr_node*>& members, bool negated) {
	return intern(detail::equality{members});
}

const detail::predicate_node* node_manager_t::make_inequal(detail::inequality::kind type, const std::vector<const detail::mathexpr_node*>& members, bool negated) {
	return intern(detail::inequality{type, members});
}

const detail::predicate_node* node_manager_t::make_congruence(const detail::mathexpr_node* mod, const std::vector<const detail::mathexpr_node*>& members, bool negated) {
	return intern(detail::congruence{mod, members});
}

const detail::predicate_node* node_manager_t::make_element_inclusion(const detail::set_node* set, const detail::mathexpr_node* elem, bool negated) {
	return intern(detail::element_inclusion{elem, set});
}

const detail::predicate_node* node_manager_t::make_set_inclusion(const detail::set_node* set, const detail::set_node* subset, bool negated) {
	return intern(detail::set_inclusion{subset, set});
}

const detail::predicate_node* node_manager_t::make_or(const detail::predicate_node* p, const detail::predicate_node* q, bool negated) {
	return intern(detail::logical_or{p, q});
}

const detail::predicate_node* node_manager_t::make_and(const detail::predicate_node* p, const detail::predicate_node* q, bool negated) {
	return intern(detail::logical_and{p, q});
}

const detail::statement_node* node_manager_t::make_sequence(const std::vector<const detail::statement_node*>& statements) {
	return intern(detail::stmt_sequence{statements});
}

const detail::statement_node* node_manager_t::make_assignment(const std::string& name, const detail::statement_node* value, detail::value_type_t expected_type) {
	return intern(detail::assignment{name, value, expected_type});
}

const detail::statement_node* node_manager_t::make_assignment(const std::string& name, const detail::mathexpr_node* value) {
	return intern(detail::assignment{name, make_expression_statement(value), detail::mathexpr_});
}

const detail::statement_node* node_manager_t::make_assignment(const std::string& name, const detail::predicate_node* value) {
	return intern(detail::assignment{name, make_expression_statement(value), detail::predicate_});
}

const detail::statement_node* node_manager_t::make_assignment(const std::string& name, const detail::set_node* value) {
	return intern(detail::assignment{name, make_expression_statement(value), detail::set_});
}

const detail::statement_node* node_manager_t::make_expression_statement(const detail::expression_value_t& expr) {
	return intern(detail::expression_statement{expr});
}

const detail::statement_node* node_manager_t::make_expression_statement(const detail::mathexpr_node* expr) {
	return intern(detail::expression_statement{expr});
}

const detail::statement_node* node_manager_t::make_expression_statement(const detail::predicate_node* expr) {
	return intern(detail::expression_statement{expr});
}

const detail::statement_node* node_manager_t::make_expression_statement(const detail::set_node* expr) {
	return intern(detail::expression_statement{expr});
}

const detail::statement_node* node_manager_t::make_conditional(const detail::statement_node* condition, const detail::statement_node* then_branch, const detail::statement_node* else_branch) {
	return intern(detail::conditional{condition, then_branch, else_branch});
}

/*const detail::statement_node* node_manager_t::make_while_loop(const detail::predicate_node* condition, const detail::statement_node* body) {
	return intern(detail::while_loop{condition, body});
}

const detail::statement_node* node_manager_t::make_for_loop(const detail::mathexpr_node* variable, const detail::set_node* iterable, const detail::statement_node* body) {
	return intern(detail::for_loop{variable, iterable, body});
}*/

const detail::statement_node* node_manager_t::make_function_definition(const std::string& name, const std::vector<std::string>& params, const detail::statement_node* body) {
	return intern(detail::function_definition{name, params, body});
}

const detail::statement_node* node_manager_t::make_function_call(uint32_t id, const std::vector<detail::expression_value_t>& params) {
	return intern(detail::function_call{id, params});
}
