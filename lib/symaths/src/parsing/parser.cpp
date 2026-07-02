#include "symaths/parsing/parser.hpp"

#include "symaths/symaths.hpp"
#include "symaths/detail/expression_node.hpp"
#include "symaths/detail/node_manager.hpp"

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
			m_errors.emplace_back(unexpected_token, std::format(R"(Expected "{}", but got "{}" instead.)", get_token_type_str(type), get_token_type_str(curr.type)));
		}
		return curr.type == type;
	}
	if (type == lexer::NONE || type == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, std::format(R"("{}" was expected)", get_token_type_str(type)));
	return false;
}

bool parser::expect(lexer::token_type type) {
	if (has_tokens()) {
		const lexer::token& token = current_token();
		if (token.type != type) {
			m_errors.emplace_back(unexpected_token, std::format(R"(Expected "{}", but got "{}" instead.)", get_token_type_str(type), get_token_type_str(token.type)));
		}
		return token.type == type;
	}
	if (type == lexer::NONE || type == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, std::format(R"("{}" was expected)", get_token_type_str(type)));
	return false;
}

bool parser::expect2(lexer::token_type type1, lexer::token_type type2) {
	if (has_tokens()) {
		const lexer::token& token = current_token();
		if (!(token.type == type1 || token.type == type2)) {
			m_errors.emplace_back(unexpected_token, std::format(R"(Expected "{}" or "{}", but got "{}" instead.)", get_token_type_str(type1), get_token_type_str(type2), get_token_type_str(token.type)));
		}
		return token.type == type1 || token.type == type2;
	}
	if (type1 == lexer::NONE || type1 == lexer::eof || type2 == lexer::NONE || type2 == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, std::format(R"("{}" or "{}" was expected)", get_token_type_str(type1), get_token_type_str(type2)));
	return false;
}

bool parser::expect_next(lexer::token_type type) {
	if (has_tokens()) {
		const lexer::token& next = *(&current_token() + 1);
		if (next.type != type) {
			m_errors.emplace_back(unexpected_token, std::format(R"(Expected "{}", but got "{}" instead.)", get_token_type_str(type), get_token_type_str(next.type)));
		}
		return next.type == type;
	}
	if (type == lexer::NONE || type == lexer::eof) {
		return true;
	}
	m_errors.emplace_back(unexpected_token, std::format(R"("{}" was expected)", get_token_type_str(type)));
	return false;
}

std::optional<lexer::token> parser::expect_identifier(const std::string& name) {
	if (!has_tokens()) return std::nullopt;
	if (current_token().value != name) return std::nullopt;

	return advance();
}

context_table parser::parse() {
	context_table table;
	while (has_tokens()) {
		table.add_entry(parse_line());
	}
	return std::move(table);
}

const detail::expression_node* parser::parse_line() {
	if (expect_identifier("func")) {
		//return parse_func(0);
	}
	else if (expect_identifier("pred")) {
		return parse_predicate(0);
	}
	return parse_expression(0);
}

const detail::expression_node* parser::parse_expression(int precedence_limit) {
	const lexer::token& prefix = advance();
	const detail::expression_node* left = parse_prefix(prefix);
	if (!left) {
		return nullptr;
	}

	while (has_tokens() && get_precedence(current_token().type) > precedence_limit) {
		const lexer::token& infix = advance();
		left = parse_infix(left, infix);
	}
	return left;
}

// Example form : pred P:x,y,z -> "3x+2y+z = 0"
const detail::expression_node* parser::parse_predicate(int precedence_limit) {
	if (!expect(lexer::identifier)) return nullptr;
	std::string name = advance().value;
	std::vector<std::string> variables;

	// Parsing variable declarations
	while (has_tokens() && current_token().type != lexer::right_arrow) {
		if (!expect(lexer::identifier)) return nullptr;
		variables.push_back(advance().value);
		if (current_token().type == lexer::open_parenthesis) {
			m_errors.emplace_back(unsupported, "Domain restriction for variables is not supported yet");
			return nullptr;
		}
		if (current_token().type == lexer::right_arrow) break;
		if (!expect(lexer::comma)) return nullptr;
	}
	advance();

	// Parsing content
	if (!expect(lexer::quote)) return nullptr;
	advance();
	const detail::expression_node* expr1 = parse_expression(0);
}

const detail::expression_node* parser::parse_prefix(const lexer::token& prefix) {
	auto& nm = current_context->node_manager();
	switch (prefix.type) {
		case lexer::number: {
			return nm.make_constant(std::stod(prefix.value));
		}
		case lexer::identifier: {
			auto func_id = detail::get_func_id(prefix.value);
			if (func_id != funcs::LEN) {
				if (consume(lexer::open_parenthesis)) {
					std::vector<const detail::expression_node*> args = parse_func_call();
					return nm.make_func(func_id, args);
				}
				return nullptr;
			}
			return nm.make_symbol(prefix.value);
		}
		case lexer::op_addition: {
			return parse_expression(0);
		}
		case lexer::op_subtraction: {
			const detail::expression_node* n = parse_expression(get_precedence(lexer::op_subtraction, true));
			return nm.make_negation(n);
		}
		case lexer::open_parenthesis: {
			const detail::expression_node* n = parse_expression(0);
			if (consume(lexer::close_parenthesis)) {
				advance();
				return n;
			}
			return nullptr;
		}
		default: {
			m_errors.emplace_back(unexpected_token, std::format("Expected a prefix token, but got \"{}\" instead.", get_token_type_str(prefix.type)));
			return nullptr;
		}
	}
}

const detail::expression_node* parser::parse_infix(const detail::expression_node* left, const lexer::token& infix) {
	auto& nm = current_context->node_manager();
	int p = get_precedence(infix.type);
	const lexer::token& prefix = m_tokens[m_index - 2];
	switch (infix.type) {
		case lexer::op_addition: {
			const detail::expression_node* right = parse_expression(p);
			return nm.make_add({left, right});
		}
		case lexer::op_subtraction: {
			const detail::expression_node* right = parse_expression(p);
			return nm.make_add({left, nm.make_negation(right)});
		}
		case lexer::op_multiplication: {
			const detail::expression_node* right = parse_expression(p);
			return nm.make_mul({left, right});
		}
		case lexer::op_division: {
			const detail::expression_node* right = parse_expression(p);
			return nm.make_div(left, right);
		}
		case lexer::op_power: {
			const detail::expression_node* right = parse_expression(p - 1); // Right-associativity : a^b^c = a^(b^c)
			return nm.make_pow(left, right);
		}
		case lexer::op_modulo: {
			m_errors.emplace_back(unsupported, "Modulo is not yet supported.");
			return nullptr;
			const detail::expression_node* right = parse_expression(p);
			return nm.make_mul({left, right});
		}
		case lexer::op_factorial: {
			m_errors.emplace_back(unsupported, "Factorial is not yet supported.");
			return nullptr;
		}
		case lexer::open_parenthesis: {
			auto func_id = detail::get_func_id(prefix.value);
			if (func_id != funcs::LEN) {
				std::vector<const detail::expression_node*> args = parse_func_call();
				return nm.make_func(func_id, args);
			}
			const detail::expression_node* right = parse_expression(0);
			advance();
			return nm.make_mul({left, right});
		}

		// Handle implicit multiplication
		case lexer::identifier: {
			auto func_id = detail::get_func_id(infix.value);
			if (func_id != funcs::LEN) {
				if (consume(lexer::open_parenthesis)) {
					std::vector<const detail::expression_node*> args = parse_func_call();
					return nm.make_mul({left, nm.make_func(func_id, args)});
				}
				return nullptr;
			}
			const detail::expression_node* right = nm.make_symbol(infix.value);
			return nm.make_mul({left, right});
		}

		default: {
			m_errors.emplace_back(unexpected_token,
				std::format("Expected an infix token, but got \"{}\" instead.", get_token_type_str(infix.type))
			);
			return nullptr;
		}
	}
}

std::vector<const detail::expression_node*> parser::parse_func_call() {
	std::vector<const detail::expression_node*> args;
	advance();
	if (current_token().type != lexer::close_parenthesis) {
		do {
			args.push_back(parse_expression(0));
			if (has_tokens() && current_token().type != lexer::comma) {
				break;
			}
			advance();
		}
		while (has_tokens());
	}
	if (consume(lexer::close_parenthesis)) {
		advance();
		return args;
	}

	return {};
}

expression sym::parse_single(const lexer& lexer) {
	parser p(lexer);
	return p.parse_line();
}

expression sym::parse_single(const std::string& input) {
	lexer l;
	l.tokenize(input);
	parser p(l);
	return p.parse_line();
}

context_table parse(const lexer& lexer) {
	parser p(lexer);
	return p.parse();
}

context_table sym::parse(const std::string& input) {
	lexer l;
	l.tokenize(input);
	parser p(l);
	return p.parse();
}

