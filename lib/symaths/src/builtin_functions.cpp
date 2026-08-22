#include "symaths/builtin_functions.hpp"

#include "symaths/differentiation.hpp"
#include "symaths/expressions_manip.hpp"
#include "symaths/expression.hpp"
#include "symaths/symbol.hpp"
#include "symaths/detail/mathexpr_node.hpp"

#include <iostream>

using namespace sym;

static std::vector<builtin_descriptor> table{};

void detail::init_builtin_functions() {
	table.push_back({
			"print", [](const std::vector<expression_value_t>& args, context_table_t* ctx) -> object {
				auto out = ctx->out_stream();

				if (out) *out << std::visit(overloaded {
					[&](const mathexpr_node* node) -> std::string { return node->resolve(out, ctx)->string(); },
					[&](const auto&) -> std::string { return object{args[0]}.string(); },
				}, args[0]) << std::endl;
				return object{};
			},
			{
				args_list_t{bool_},
				args_list_t{number_},
				args_list_t{mathexpr_},
				args_list_t{predicate_},
				args_list_t{set_}
			},
			null
		});
	table.push_back({
			"reduce", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				return object{reduce(expr).root};
			},
			{ args_list_t{mathexpr_} },
			mathexpr_
		});
	table.push_back({
			"expand", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				return object{expand(expr).root};
			},
			{ args_list_t{mathexpr_} },
			mathexpr_
		});
	table.push_back({
			"sort", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				return object{sort(expr).root};
			},
			{ args_list_t{mathexpr_} },
			mathexpr_
		});
	table.push_back({
			"differentiate", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				sym::symbol wrt(std::get<const mathexpr_node*>(args[1]));
				return object{differentiate(expr, wrt).root};
			},
			{ args_list_t{mathexpr_, mathexpr_} },
			mathexpr_
		});
}


static std::vector<builtin_descriptor>& builtins() {
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
