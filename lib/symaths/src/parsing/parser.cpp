#include "symaths/parsing/parser.hpp"

#include "symaths/base_functions.hpp"
#include "symaths/builtin_functions.hpp"
#include "symaths/symaths.hpp"
#include "symaths/detail/mathexpr_node.hpp"
#include "symaths/detail/node_manager.hpp"

#include <algorithm>
#include <array>
#include <format>

using namespace sym;

int get_precedence(lexer::token_type type, bool unary_context = false) {
	switch (type) {
		case lexer::comma:
		case lexer::close_parenthesis:
		case lexer::equal:
		case lexer::inferior:
		case lexer::superior:
		case lexer::infequal:
		case lexer::supequal:
		case lexer::not_equal:
			return 0;

		case lexer::number:
		case lexer::identifier:
			return 9999;

		case lexer::op_addition:
		case lexer::op_subtraction:
			if (unary_context)
				return 40;
			return 20;

		case lexer::op_multiplication:
		case lexer::op_division:
			return 30;

		case lexer::op_power:
			return 50;

		case lexer::op_modulo:
			return 60;

		case lexer::op_factorial:
			return 70;

		case lexer::open_parenthesis:
			return 80;

		default: return -1;
	}
}

std::array<std::string, 9> reserved_identifiers = {
	"and",
	"delete",
	"else",
	"expr",
	"func",
	"if",
	"or",
	"pred",
	"set"
};


parser::parser(const lexer& lexer, context_table_t* table) : m_tokens(lexer.tokens()) {
	if (table) m_variables = table;
	else m_variables = &current_context->context_table();
}

parser::parser(lexer&& lexer, context_table_t* table) : m_tokens(std::move(lexer).tokens()) {
	if (table) m_variables = table;
	else m_variables = &current_context->context_table();
}

bool parser::has_errors() const {
	return !m_errors.empty();
}

exception parser::build_error() const {
	std::string err_msg;
	for (auto& error : m_errors) {
		err_msg += error.string() + "\n";
	}
	if (!err_msg.empty()) err_msg.pop_back();
	return exception{err_msg};
}

bool parser::has_tokens() const {
	return m_index < m_tokens.size() && m_tokens[m_index].type != lexer::eof;
}

const lexer::token& parser::current_token() const {
	return m_tokens[m_index];
}

const lexer::token& parser::advance() {
	return m_tokens[m_index++];
}

bool parser::consume(lexer::token_type type) {
	if (has_tokens()) {
		const lexer::token& curr = current_token();
		if (curr.type != type) {
			m_errors.emplace_back(unexpected_token, curr, std::format(R"(Expected "{}", but got "{}" instead.)", get_token_type_str(type), get_token_type_str(curr.type)));
			return false;
		}
		advance();
		return true;
	}
	if (type == lexer::NONE || type == lexer::eof) {
		advance();
		return true;
	}
	m_errors.emplace_back(unexpected_token, m_tokens[m_index-1], std::format(R"("{}" was expected.)", get_token_type_str(type)));
	return false;
}

bool parser::expect(lexer::token_type type) {
	if (has_tokens()) {
		const lexer::token& token = current_token();
		if (token.type != type) {
			m_errors.emplace_back(unexpected_token, current_token(), std::format(R"(Expected "{}", but got "{}" instead.)", get_token_type_str(type), get_token_type_str(token.type)));
		}
		return token.type == type;
	}
	if (type == lexer::NONE || type == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, current_token(), std::format(R"("{}" was expected.)", get_token_type_str(type)));
	return false;
}

bool parser::expect2(lexer::token_type type1, lexer::token_type type2) {
	if (has_tokens()) {
		const lexer::token& token = current_token();
		if (!(token.type == type1 || token.type == type2)) {
			m_errors.emplace_back(unexpected_token, current_token(), std::format(R"(Expected "{}" or "{}", but got "{}" instead.)", get_token_type_str(type1), get_token_type_str(type2), get_token_type_str(token.type)));
		}
		return token.type == type1 || token.type == type2;
	}
	if (type1 == lexer::NONE || type1 == lexer::eof || type2 == lexer::NONE || type2 == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, current_token(), std::format(R"("{}" or "{}" was expected.)", get_token_type_str(type1), get_token_type_str(type2)));
	return false;
}

bool parser::expect_next(lexer::token_type type) {
	if (has_tokens()) {
		const lexer::token& next = *(&current_token() + 1);
		if (next.type != type) {
			m_errors.emplace_back(unexpected_token, current_token(), std::format(R"(Expected "{}", but got "{}" instead.)", get_token_type_str(type), get_token_type_str(next.type)));
		}
		return next.type == type;
	}
	if (type == lexer::NONE || type == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, current_token(), std::format(R"("{}" was expected.)", get_token_type_str(type)));
	return false;
}

std::optional<lexer::token> parser::expect_identifier(const std::string& name) {
	if (!has_tokens()) return std::nullopt;
	if (current_token().value != name) return std::nullopt;

	return advance();
}

const detail::statement_node* parser::parse() {
	auto& nm = current_context->node_manager();
	std::vector<const detail::statement_node*> stmts;
	while (has_tokens()) {
		stmts.push_back(parse_stmt(m_variables, detail::null));
	}
	if (stmts.empty()) return nullptr;
	if (has_errors() || std::ranges::any_of(stmts, [&](const detail::statement_node* node) { return !node; })) {
		return nm.make_expression_statement(build_error());
	}
	if (stmts.size() == 1) return stmts.front();
	return nm.make_sequence(stmts);
}

const detail::statement_node* parser::parse_stmt(context_table_t* table, detail::value_type_t ctx_type) {
	m_context_type.push_back(ctx_type);

	const detail::statement_node* out = nullptr;
	auto& nm = current_context->node_manager();

	if (current_token().type == lexer::open_parenthesis) {

		// First try parsing as mathexpr
		size_t last_index = m_index;
		size_t last_errsize = m_errors.size();
		const detail::mathexpr_node* node = parse_mathexpr(0);
		if (node) {
			return nm.make_expression_statement(node);
		}
		m_index = last_index;
		m_errors.resize(last_errsize);

		// Fallback : parse statement
		advance();
		out = parse_stmt(table, m_context_type.back());
		if (!consume(lexer::close_parenthesis)) return nullptr;
	}

	else if (expect_identifier("expr")) {
		std::string name = parse_name();
		if (!consume(lexer::colon)) return nullptr;
		const detail::statement_node* rhs = parse_stmt(table, detail::mathexpr_);
		if (rhs && rhs->return_type() != detail::mathexpr_) {
			m_errors.push_back({type_error, current_token(), "Expected mathexpr for \"expr " + name + "\"."});
			return nullptr;
		}
		out = nm.make_assignment(name, rhs, detail::mathexpr_);
		m_variables->add_uninitialized_entry(name, detail::mathexpr_);
	}

	else if (expect_identifier("func")) {
		//return parse_func(0);
	}

	else if (expect_identifier("pred")) {
		std::string name = parse_name();
		if (!consume(lexer::colon)) return nullptr;
		const detail::statement_node* rhs = parse_stmt(table, detail::predicate_);
		if (rhs && rhs->return_type() != detail::predicate_) {
			m_errors.emplace_back(type_error, current_token(), "Expected predicate for \"pred " + name + "\".");
			return nullptr;
		}
		out = nm.make_assignment(name, rhs, detail::predicate_);
		m_variables->add_uninitialized_entry(name, detail::predicate_);
	}

	else if (expect_identifier("set")) {
		std::string name = parse_name();
		if (!consume(lexer::colon)) return nullptr;
		const detail::statement_node* rhs = parse_stmt(table, detail::set_);
		if (rhs && rhs->return_type() != detail::set_) {
			m_errors.emplace_back(type_error, current_token(), "Expected set for \"set " + name + "\".");
			return nullptr;
		}
		out = nm.make_assignment(name, rhs, detail::set_);
		m_variables->add_uninitialized_entry(name, detail::set_);
	}

	else if (uint32_t fid = get_builtin_id(current_token().value); fid != UINT32_MAX) {
		advance();
		out = parse_statement_builtin(table, fid);
		if (out->return_type() == detail::mathexpr_) {
			detail::function_call node = std::get<detail::function_call>(out->p_data);
			out = nm.make_expression_statement(nm.make_builtin_call(node.id, node.args));
		}
	}

	else {
		switch (m_context_type.back()) {
			case detail::bool_:
				out = nm.make_expression_statement(parse_bool().value());
				break;

			case detail::number_:
				out = nm.make_expression_statement(parse_number().value());
				break;

			case detail::null:
			case detail::mathexpr_: {
				if (current_token().type == lexer::open_sqbracket) {
					out = nm.make_expression_statement(parse_predicate(0));
				} else {
					out = nm.make_expression_statement(parse_mathexpr(0));
				}
				break;
			}

			case detail::predicate_:
				out = nm.make_expression_statement(parse_predicate(0));
				break;

			case detail::set_:
				// TODO : implement parse_set()
				break;
		}
	}

	if (has_tokens() && current_token().type == lexer::semicolon) advance();

	m_context_type.pop_back();
	return out;
}


std::optional<bool> parser::parse_bool() {
	const lexer::token& val = advance();

	if (val.type == lexer::identifier) {
		if (val.value == "true") {
			return true;
		}
		if (val.value == "false") {
			return false;
		}
	}
	m_errors.emplace_back(unexpected_token, val, std::format(R"(Expected "bool" but got "{}".)", val.value));
	return std::nullopt;
}
std::optional<number> parser::parse_number() {
	const lexer::token& val = advance();
	const detail::mathexpr_node* expr = parse_mathexpr(val.type);

	if (!expr) return std::nullopt;
	if (!expr->is_ground()) return std::nullopt;
	return expr->eval(nullptr);
}

const detail::mathexpr_node* parser::parse_mathexpr(int precedence_limit) {
	const lexer::token& prefix = advance();
	const detail::mathexpr_node* left = parse_prefix(prefix);
	if (!left) {
		return nullptr;
	}

	while (left && has_tokens() && get_precedence(current_token().type) > precedence_limit) {
		const lexer::token& infix = advance();
		left = parse_infix(left, infix);
	}
	return left;
}

// Example form : pred P: [x,y,z] -> "3x+2y+z = 0"
const detail::predicate_node* parser::parse_predicate(int precedence_limit) {
	if (!consume(lexer::open_sqbracket)) return nullptr;

	std::vector<std::string> variables;
	if (current_token().type != lexer::close_sqbracket) {
		do {
			if (!expect(lexer::identifier)) return nullptr;
			variables.push_back(advance().value);
			if (current_token().type == lexer::close_sqbracket) break;
			if (!expect(lexer::comma)) return nullptr;
		} while (has_tokens());
	}
	if (!consume(lexer::close_sqbracket)) return nullptr;

	if (!consume(lexer::right_arrow)) return nullptr;

	if (!consume(lexer::quote)) {
		m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to start predicate body.)");
		return nullptr;
	}

	// Parse left math expression
	const detail::mathexpr_node* left = parse_mathexpr(0);
	if (!left) return nullptr;

	// Parse comparison operator and build predicate
	auto& nm = current_context->node_manager();
	if (has_tokens()) {
		switch (current_token().type) {
			case lexer::equal: {
				advance();
				const detail::mathexpr_node* right = parse_mathexpr(0);
				if (!right) return nullptr;
				if (!consume(lexer::quote)) {
					m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to end predicate body.)");
					return nullptr;
				}
				return nm.make_equal({left, right});
			}
			case lexer::inferior: {
				advance();
				const detail::mathexpr_node* right = parse_mathexpr(0);
				if (!right) return nullptr;
				if (!consume(lexer::quote)) {
					m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to end predicate body.)");
					return nullptr;
				}
				return nm.make_inequal(detail::inequality::LESS, {left, right});
			}
			case lexer::superior: {
				advance();
				const detail::mathexpr_node* right = parse_mathexpr(0);
				if (!right) return nullptr;
				if (!consume(lexer::quote)) {
					m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to end predicate body.)");
					return nullptr;
				}
				return nm.make_inequal(detail::inequality::GREATER, {left, right});
			}
			case lexer::infequal: {
				advance();
				const detail::mathexpr_node* right = parse_mathexpr(0);
				if (!right) return nullptr;
				if (!consume(lexer::quote)) {
					m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to end predicate body.)");
					return nullptr;
				}
				return nm.make_inequal(detail::inequality::LESS_EQUAL, {left, right});
			}
			case lexer::supequal: {
				advance();
				const detail::mathexpr_node* right = parse_mathexpr(0);
				if (!right) return nullptr;
				if (!consume(lexer::quote)) {
					m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to end predicate body.)");
					return nullptr;
				}
				return nm.make_inequal(detail::inequality::GREATER_EQUAL, {left, right});
			}
			case lexer::not_equal: {
				advance();
				const detail::mathexpr_node* right = parse_mathexpr(0);
				if (!right) return nullptr;
				if (!consume(lexer::quote)) {
					m_errors.emplace_back(unexpected_token, current_token(), R"(Expected '"' to end predicate body.)");
					return nullptr;
				}
				m_errors.emplace_back(unsupported, current_token(), "Not equal predicate is not supported yet.");
				return nullptr;
			}
			default:
				m_errors.emplace_back(unexpected_token, current_token(), "Expected a comparison operator in predicate.");
				return nullptr;
		}
	}

	m_errors.emplace_back(unexpected_token, current_token(), "Unexpected end of predicate.");
	return nullptr;
}

std::string parser::parse_name() {
	auto& nm = current_context->node_manager();
	if (expect(lexer::identifier)) {
		std::string name = current_token().value;
		advance();
		return name;
	}
	return "";
}

const detail::mathexpr_node* parser::parse_prefix(const lexer::token& prefix) {
	auto& nm = current_context->node_manager();
	switch (prefix.type) {
		case lexer::number: {
			return nm.make_constant(std::stod(prefix.value));
		}
		case lexer::identifier: {
			auto func_id = detail::get_func_id(prefix.value);
			if (func_id != funcs::LEN) {
				if (consume(lexer::open_parenthesis)) {
					std::vector<const detail::mathexpr_node*> args = parse_func_call();
					if (std::ranges::any_of(args, [](const auto& node) { return !node; })) {
						return nullptr;
					}
					return nm.make_func(func_id, args);
				}
				return nullptr;
			}
			auto builtin_id = get_builtin_id(prefix.value);
			if (builtin_id != UINT32_MAX) {
				auto rt = get_builtin(builtin_id).return_type;
				if (rt == detail::mathexpr_) {
					return parse_mathexpr_builtin(&current_context->context_table(), builtin_id); // TODO add own context table.
				}
				m_errors.emplace_back(type_error, prefix, std::format(R"(Expected return type "mathexpr" but got "{}" instead.)", detail::value_type_string(rt)));
			}
			m_variables->add_uninitialized_entry(prefix.value, detail::mathexpr_);
			return nm.make_symbol(prefix.value);
		}
		case lexer::op_addition: {
			return parse_mathexpr(0);
		}
		case lexer::op_subtraction: {
			const detail::mathexpr_node* n = parse_mathexpr(get_precedence(lexer::op_subtraction, true));
			return n ? nm.make_negation(n) : nullptr;
		}
		case lexer::open_parenthesis: {
			const detail::mathexpr_node* n = parse_mathexpr(0);
			if (consume(lexer::close_parenthesis)) {
				return n ? n : nullptr;
			}
			m_errors.emplace_back(unexpected_token, prefix, std::format("Expected a closing parenthesis, but got none.", get_token_type_str(prefix.type)));
			return nullptr;
		}
		default: {
			m_errors.emplace_back(unexpected_token, prefix, std::format("Expected a prefix token, but got \"{}\" instead.", get_token_type_str(prefix.type)));
			return nullptr;
		}
	}
}

const detail::mathexpr_node* parser::parse_infix(const detail::mathexpr_node* left, const lexer::token& infix) {
	auto& nm = current_context->node_manager();
	int p = get_precedence(infix.type);
	const lexer::token& prefix = m_tokens[m_index - 2];
	switch (infix.type) {
		case lexer::op_addition: {
			const detail::mathexpr_node* right = parse_mathexpr(p);
			return right ? nm.make_add({left, right}) : nullptr;
		}
		case lexer::op_subtraction: {
			const detail::mathexpr_node* right = parse_mathexpr(p);
			return right ? nm.make_add({left, nm.make_negation(right)}) : nullptr;
		}
		case lexer::op_multiplication: {
			const detail::mathexpr_node* right = parse_mathexpr(p);
			return right ? nm.make_mul({left, right}) : nullptr;
		}
		case lexer::op_division: {
			const detail::mathexpr_node* right = parse_mathexpr(p);
			return right ? nm.make_div(left, right) : nullptr;
		}
		case lexer::op_power: {
			const detail::mathexpr_node* right = parse_mathexpr(p - 1); // Right-associativity : a^b^c = a^(b^c)
			return right ? nm.make_pow(left, right) : nullptr;
		}
		case lexer::op_modulo: {
			m_errors.emplace_back(unsupported, infix, "Modulo is not yet supported.");
			return nullptr;
			//const detail::expression_node* right = parse_expression(p);
			//return nm.make_mul({left, right});
		}
		case lexer::op_factorial: {
			m_errors.emplace_back(unsupported, infix, "Factorial is not yet supported.");
			return nullptr;
		}
		case lexer::open_parenthesis: {
			auto func_id = detail::get_func_id(prefix.value);
			if (func_id != funcs::LEN) {
				advance();
				std::vector<const detail::mathexpr_node*> args = parse_func_call();
				if (std::ranges::any_of(args, [](const auto& node) { return !node; })) {
					return nullptr;
				}
				return nm.make_func(func_id, args);
			}
			auto builtin_id = get_builtin_id(prefix.value);
			if (builtin_id != UINT32_MAX) {
				const detail::mathexpr_node* n = parse_mathexpr_builtin(&current_context->context_table(), builtin_id);
				return n ? nm.make_mul({left, n}) : nullptr;
			}
			const detail::mathexpr_node* right = parse_mathexpr(0);
			advance();
			return right ? nm.make_mul({left, right}) : nullptr;
		}

		/*
		 * When parsing for example "(3x+3)", the infix token will be lexer::close_parenthesis. In this case we'll exit
		 * mathematical expression parsing context early
		 */
		case lexer::semicolon:
		case lexer::close_parenthesis: {
			return left;
		}

		// Handle implicit multiplication
		case lexer::identifier: {
			auto func_id = detail::get_func_id(infix.value);
			if (func_id != funcs::LEN) {
				if (consume(lexer::open_parenthesis)) {
					std::vector<const detail::mathexpr_node*> args = parse_func_call();
					if (std::ranges::any_of(args, [](const auto& node) { return !node; })) {
						return nullptr;
					}
					return nm.make_mul({left, nm.make_func(func_id, args)});
				}
				return nullptr;
			}
			auto builtin_id = get_builtin_id(infix.value);
			if (builtin_id != UINT32_MAX) {
				auto rt = get_builtin(builtin_id).return_type;
				if (rt == detail::mathexpr_) {
					const detail::mathexpr_node* n = parse_mathexpr_builtin(&current_context->context_table(), builtin_id);
					return n ? nm.make_mul({left, n}) : nullptr;; // TODO add own context table.
				}
				m_errors.emplace_back(type_error, prefix, std::format(R"(Expected return type "mathexpr" but got "{}" instead.)", detail::value_type_string(rt)));
				return nullptr;
			}
			m_index--;
			const detail::mathexpr_node* right = parse_mathexpr(30); // precedence of multiplication = 30
			return right ? nm.make_mul({left, right}) : nullptr;
		}

		default: {
			m_errors.emplace_back(unexpected_token, infix,
				std::format("Expected an infix token, but got \"{}\" instead.", get_token_type_str(infix.type))
			);
			return nullptr;
		}
	}
}

std::vector<const detail::mathexpr_node*> parser::parse_func_call() {
	std::vector<const detail::mathexpr_node*> args;
	if (current_token().type != lexer::close_parenthesis) {
		do {
			args.push_back(parse_mathexpr(0));
			if (has_tokens() && current_token().type != lexer::comma) {
				break;
			}
			advance();
		}
		while (has_tokens());
	}
	if (consume(lexer::close_parenthesis)) {
		return args;
	}

	return {};
}

const detail::mathexpr_node* parser::parse_mathexpr_builtin(context_table_t* ctx, uint32_t builtin_id) {
	if (!consume(lexer::open_parenthesis)) return nullptr;

	auto& nm = current_context->node_manager();
	const auto& desc = get_builtin(builtin_id);

	std::vector<detail::expression_value_t> args;
	while (has_tokens() && current_token().type != lexer::close_parenthesis) {
		const detail::statement_node* stmt_node = parse_stmt(ctx, detail::null);
		std::optional<detail::expression_value_t> value = std::visit(overloaded {
			[&](const detail::expression_statement& stmt) -> std::optional<detail::expression_value_t> { return stmt.expr; },
			[&](const detail::function_call& stmt) -> std::optional<detail::expression_value_t> {
				auto func = get_builtin(stmt.id);
				return func.handler(args, ctx).root;
			},
			[&](const auto&) -> std::optional<detail::expression_value_t> {
				m_errors.emplace_back(unexpected_token, current_token(), "Unexpected statement.");
				return std::nullopt;
			},
		}, stmt_node->p_data);
		if (!value.has_value()) return nullptr;
		args.push_back(value.value());

		if (!has_tokens()) break;
		if (current_token().type != lexer::close_parenthesis) consume(lexer::comma);
	}

	if (!consume(lexer::close_parenthesis)) return nullptr;

	auto candidates = get_candidates(desc, args.size());

	// Type validation
	for (size_t i = 0; i < args.size(); ++i) {
		auto& arg = args[i];
		for (size_t j = 0; j < candidates.size(); ++j) {
			auto& candidate = candidates[j];
			if (arg.index() != candidate[i]) {
				candidates.erase(candidates.begin() + j);
			}
		}
	}

	if (candidates.empty()) {
		std::string msg = std::format("No viable candidate for call to \"{}\". Possible candidates are :\n", desc.name);
		for (auto& candidate : desc.arg_types) {
			std::string cdt = std::format("  - {}(", desc.name);
			for (auto& arg : candidate) {
				cdt += detail::value_type_string(arg);
				if (&arg != &candidate.back()) cdt += ", ";
			}
			cdt += ")\n";
			msg += cdt;
		}
		m_errors.emplace_back(invalid_signature, current_token(), msg);
		return nullptr;
	}
	if (candidates.size() > 1) {
		m_errors.emplace_back(invalid_signature, current_token(), std::format("Too many candidates for call to \"{}\".", desc.name));
		return nullptr;
	}

	return nm.make_builtin_call(builtin_id, args);
}

const detail::statement_node* parser::parse_statement_builtin(context_table_t* ctx, uint32_t builtin_id) {
	if (!consume(lexer::open_parenthesis)) return nullptr;

	auto& nm = current_context->node_manager();
	const auto& desc = get_builtin(builtin_id);

	std::vector<detail::expression_value_t> args;
	while (has_tokens() && current_token().type != lexer::close_parenthesis) {
		const detail::statement_node* stmt_node = parse_stmt(ctx, detail::null);
		std::optional<detail::expression_value_t> value = std::visit(overloaded {
			[&](const detail::expression_statement& stmt) -> std::optional<detail::expression_value_t> { return stmt.expr; },
			[&](const detail::function_call& stmt) -> std::optional<detail::expression_value_t> {
				auto func = get_builtin(stmt.id);
				return func.handler(stmt.args, ctx).root;
			},
			[&](const auto&) -> std::optional<detail::expression_value_t> {
				m_errors.emplace_back(unexpected_token, current_token(), "Unexpected statement.");
				return std::nullopt;
			},
		}, stmt_node->p_data);
		if (!value.has_value()) return nullptr;
		args.push_back(value.value());

		if (!has_tokens()) break;
		if (current_token().type != lexer::close_parenthesis) consume(lexer::comma);
	}

	if (!consume(lexer::close_parenthesis)) return nullptr;

	auto candidates = get_candidates(desc, args.size());

	// Type validation
	for (size_t i = 0; i < args.size(); ++i) {
		auto& arg = args[i];
		for (size_t j = 0; j < candidates.size(); ++j) {
			auto& candidate = candidates[j];
			if (arg.index() != candidate[i]) {
				candidates.erase(candidates.begin() + j);
				j--;
			}
		}
	}

	if (candidates.empty()) {
		std::string msg = std::format("No viable candidate for call to \"{}\". Possible candidates are :\n", desc.name);
		for (auto& candidate : desc.arg_types) {
			std::string cdt = std::format("  - {}(", desc.name);
			for (auto& arg : candidate) {
				cdt += detail::value_type_string(arg);
				if (&arg != &candidate.back()) cdt += ", ";
			}
			cdt += ")\n";
			msg += cdt;
		}
		m_errors.emplace_back(invalid_signature, current_token(), msg);
		return nullptr;
	}
	if (candidates.size() > 1) {
		m_errors.emplace_back(invalid_signature, current_token(), std::format("Too many candidates for call to \"{}\".", desc.name));
		return nullptr;
	}

	return nm.make_function_call(builtin_id, args);
}

program sym::parse_single(const lexer& lexer) {
	parser p(lexer);
	auto out =  p.parse_stmt(&current_context->context_table(), detail::null);
	return out;
}

program sym::parse_single(const std::string& input) {
	lexer l;
	l.tokenize(input);
	parser p(l);
	return p.parse_stmt(&current_context->context_table(), detail::null);
}

program sym::parse(const lexer& lexer) {
	parser p(lexer);
	return p.parse();
}

program sym::parse(const std::string& input) {
	lexer l;
	l.tokenize(input);
	parser p(l);
	return p.parse();
}

expression sym::parse_expression(const lexer& lexer) {
	parser p(lexer);
	auto out = p.parse_stmt(&current_context->context_table(), detail::null);
	return std::get<const detail::mathexpr_node*>(out->eval(&std::cout, &current_context->context_table()).value());
}

expression sym::parse_expression(const std::string& input) {
	lexer l;
	l.tokenize(input);
	parser p(l);
	auto out = p.parse_stmt(&current_context->context_table(), detail::null);
	return std::get<const detail::mathexpr_node*>(out->eval(&std::cout, &current_context->context_table()).value());
}