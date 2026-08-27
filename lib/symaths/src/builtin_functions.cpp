#include "symaths/builtin_functions.hpp"

#include "symaths/differentiation.hpp"
#include "symaths/expressions_manip.hpp"
#include "symaths/expression.hpp"
#include "symaths/symbol.hpp"
#include "symaths/detail/mathexpr_node.hpp"

#include <iostream>

using namespace sym;

void detail::init_builtin_functions(std::vector<internal_func_descriptor>& out) {
	out.push_back({
			"print", [](const std::vector<expression_value_t>& args, context_table_t* ctx) -> object {
				auto out = ctx->out_stream();

				if (out) *out << std::visit(overloaded {
					[&](const mathexpr_node* node) -> std::string { return node->resolve(out, ctx)->string(nullptr, true, ctx); },
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
	out.push_back({
			"reduce", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				return object{reduce(expr).root};
			},
			{ args_list_t{mathexpr_} },
			mathexpr_
		});
	out.push_back({
			"expand", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				return object{expand(expr).root};
			},
			{ args_list_t{mathexpr_} },
			mathexpr_
		});
	out.push_back({
			"sort", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				return object{sort(expr).root};
			},
			{ args_list_t{mathexpr_} },
			mathexpr_
		});
	out.push_back({
			"differentiate", [](const std::vector<expression_value_t>& args, context_table_t*) -> object {
				expression expr(std::get<const mathexpr_node*>(args[0]));
				sym::symbol wrt(std::get<const mathexpr_node*>(args[1]));
				return object{differentiate(expr, wrt).root};
			},
			{ args_list_t{mathexpr_, mathexpr_} },
			mathexpr_
		});
}