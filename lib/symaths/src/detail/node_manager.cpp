#include "symaths/detail/node_manager.hpp"

using namespace sym;

std::size_t expr_node_hash::operator()(const node_key<detail::expression_node>& k) const {
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
				h ^= std::hash<const detail::expression_node*>{}(c) + 0x9e3779b9;
		}

		else if constexpr (std::is_same_v<T, detail::power>) {
			h ^= std::hash<const detail::expression_node*>{}(x.base);
			h ^= std::hash<const detail::expression_node*>{}(x.exponent);
		}

		return h;
	}, k.data);
}

std::size_t pred_node_hash::operator()(const node_key<detail::predicate_node>& k) const {
	return std::visit([](const auto& x) {
		using T = std::decay_t<decltype(x)>;
		size_t h = typeid(T).hash_code();

		// TODO : do things

		return h;
	}, k.data);
}


const detail::expression_node* node_manager_t::intern(detail::expression_node::internal_data_t data) {
	node_key<detail::expression_node> key{data};

	auto it = expr_table.find(key);
	if (it != expr_table.end())
		return it->second;

	auto n = std::make_unique<detail::expression_node>();
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

template<typename T>
std::vector<const detail::expression_node*> flatten(const std::vector<const detail::expression_node*>& args) {
	std::vector<const detail::expression_node*> flat;
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

detail::expression_node::internal_data_t reduce_negations(detail::negation arg) {
	// 2 negations = +
	if (std::holds_alternative<detail::negation>(arg.child->p_data)) {
		return std::get<detail::negation>(arg.child->p_data).child->p_data;
	}
	return arg;
}

const detail::expression_node* node_manager_t::make_symbol(const std::string& name) {
	return intern(detail::symbol{name});
}

const detail::expression_node* node_manager_t::make_constant(const number& v) {
	return intern(detail::constant{v});
}

const detail::expression_node* node_manager_t::make_constant(double v) {
	return intern(detail::constant{numbers::real{v}});
}

const detail::expression_node* node_manager_t::make_negation(const detail::expression_node* node) {
	return intern(reduce_negations(detail::negation{node}));
}

const detail::expression_node* node_manager_t::make_add(const std::vector<const detail::expression_node*>& operands) {
	return intern(detail::addition{flatten<detail::addition>(operands)});
}

const detail::expression_node* node_manager_t::make_mul(const std::vector<const detail::expression_node*>& operands) {
	return intern(detail::multiplication{flatten<detail::multiplication>(operands)});
}

const detail::expression_node* node_manager_t::make_div(const detail::expression_node* a, const detail::expression_node* b) {
	return make_mul({a, make_pow(b, make_constant(-1))});
}

const detail::expression_node* node_manager_t::make_pow(const detail::expression_node* b, const detail::expression_node* e) {
	return intern(detail::power{b, e});
}

const detail::expression_node* node_manager_t::make_func(uint32_t f_id, const std::vector<const detail::expression_node*>& args) {
	return intern(detail::function_call{f_id, args});
}

const detail::expression_node* node_manager_t::make_func(funcs::builtin_fn_id f_id, const std::vector<const detail::expression_node*>& args) {
	return intern(detail::function_call{static_cast<uint32_t>(f_id), args});
}