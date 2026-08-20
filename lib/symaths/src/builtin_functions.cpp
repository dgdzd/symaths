#include "symaths/builtin_functions.hpp"

#include "symaths/differentiation.hpp"
#include "symaths/expressions_manip.hpp"
#include "symaths/expression.hpp"
#include "symaths/symbol.hpp"
#include "symaths/detail/mathexpr_node.hpp"

#include <iostream>

using namespace sym;

static std::vector<builtin_descriptor>& builtins() {
	static std::vector<builtin_descriptor> table = {
		{
			"reduce", [](const std::vector<detail::expression_value_t>& args, context_table_t*) -> detail::expression_value_t {
				expression expr(std::get<const detail::mathexpr_node*>(args[0]));
				return reduce(expr).root;
			},
			{ args_list_t{detail::mathexpr_} }
		},
		{
			"expand", [](const std::vector<detail::expression_value_t>& args, context_table_t*) -> detail::expression_value_t {
				expression expr(std::get<const detail::mathexpr_node*>(args[0]));
				return expand(expr).root;
			},
			{ args_list_t{ detail::mathexpr_ } }
		},
		{
			"sort", [](const std::vector<detail::expression_value_t>& args, context_table_t*) -> detail::expression_value_t {
				expression expr(std::get<const detail::mathexpr_node*>(args[0]));
				return sort(expr).root;
			},
			{ args_list_t{ detail::mathexpr_ } }
		},
		{
			"differentiate", [](const std::vector<detail::expression_value_t>& args, context_table_t*) -> detail::expression_value_t {
				expression expr(std::get<const detail::mathexpr_node*>(args[0]));
				symbol wrt(std::get<const detail::mathexpr_node*>(args[1]));
				return differentiate(expr, wrt).root;
			},
			{ args_list_t{ detail::mathexpr_, detail::mathexpr_ } }
		},
	};
	return table;
}

uint32_t sym::get_builtin_id(const std::string& name) {
	auto& table = builtins();
	for (uint32_t i = 0; i < table.size(); ++i) {
		if (table[i].name == name) return i;
	}
	return UINT32_MAX;
}

uint32_t sym::builtin_count() {
	return builtins().size();
}

const builtin_descriptor& sym::get_builtin(uint32_t id) {
	return builtins()[id];
}

std::vector<args_list_t> sym::get_candidates(const builtin_descriptor& builtin, size_t arguments_count) {
	std::vector<args_list_t> candidates;
	for (auto& args : builtin.arg_types) {
		if (args.size() == arguments_count) {
			candidates.push_back(args);
		}
	}
	return candidates;
}
