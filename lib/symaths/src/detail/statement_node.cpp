#include "symaths/detail/statement_node.hpp"

#include "symaths/context_table.hpp"
#include "symaths/utils/helpers.hpp"

using namespace sym::detail;


std::optional<expression_value_t> statement_node::eval(context_table_t* ctx) const {
	return std::visit(overloaded {
		[&](const stmt_sequence& seq) -> std::optional<expression_value_t> {
			for (auto& stmt : seq.statements) {
				auto out = stmt->eval(ctx);
			}
			return std::nullopt;
		},
		[&](const expression_statement& stmt) -> std::optional<expression_value_t> {
			if (auto* mp = std::get_if<const mathexpr_node*>(&stmt.expr)) {
				return expression_value_t{(*mp)->resolve_builtins()};
			}
			return stmt.expr;
		},
		[&](const assignment& stmt) -> std::optional<expression_value_t> {
			auto val = stmt.value->eval(ctx).value();
			if (auto* mp = std::get_if<const mathexpr_node*>(&val)) {
				val = expression_value_t{(*mp)->resolve_builtins()};
			}
			ctx->add_named_entry(stmt.varname, val);
			return val;
		},
		[&](const conditional& stmt) -> std::optional<expression_value_t> {
			bool cond = stmt.condition->eval(ctx).has_value(); // TODO : For now, just check if condition has an expression value.
			if (cond) return stmt.then_branch->eval(ctx);
			if (stmt.else_branch) return stmt.else_branch->eval(ctx);
			return std::nullopt;
		},
		[&](const function_definition& stmt) -> std::optional<expression_value_t> {
			// TODO : define function
			return std::nullopt;
		},
		[&](const auto&) -> std::optional<expression_value_t> {
			return std::nullopt;
		}
	}, p_data);
}

value_type_t statement_node::return_type() const {
	return std::visit(overloaded {
		[](const assignment& a) -> value_type_t {
			return a.expected_type;
		},
		[](const expression_statement& es) -> value_type_t {
			return static_cast<value_type_t>(es.expr.index());
		},
		[](const stmt_sequence&) -> value_type_t {
			return null;
		},
		[](const conditional& c) -> value_type_t {
			if (c.then_branch && c.else_branch) {
				auto rt = c.then_branch->return_type();
				if (rt == c.else_branch->return_type()) {
					return rt;
				}
				return null;
			}
			if (c.then_branch) {
				return c.then_branch->return_type();
			}
			return null;
		},
		[](const function_definition&) -> value_type_t {
			return null;
		},
		[](const auto&) -> value_type_t {
			return null;
		},
	}, p_data);
}
