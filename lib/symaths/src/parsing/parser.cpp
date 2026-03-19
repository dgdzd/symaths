#include "symaths/parsing/parser.hpp"

#include "symaths/symaths.hpp"
#include "symaths/detail/nodes.hpp"

#include <format>
#include <map>

using namespace sym;

int get_precedence(lexer::token_type type, bool unary_context = false) {
	switch (type) {
		case lexer::comma:
		case lexer::close_parenthesis:
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

		case lexer::open_parenthesis:
			return 70;

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

const detail::node* parser::parse(int precedence_limit) {
	return parse_expression(precedence_limit);
}

const detail::node* parser::parse_expression(int precedence_limit) {
	const lexer::token& prefix = advance();
	const detail::node* left = parse_prefix(prefix);
	if (!left) {
		return nullptr;
	}

	while (has_tokens() && get_precedence(current_token().type) > precedence_limit) {
		const lexer::token& infix = advance();
		left = parse_infix(left, infix);
	}
	return left;
}

const detail::node* parser::parse_prefix(const lexer::token& prefix) {
	auto& nm = current_context->node_manager();
	switch (prefix.type) {
		case lexer::number: {
			return nm.make_constant(std::stod(prefix.value));
		}
		case lexer::identifier: {
			auto func_id = detail::get_func_id(prefix.value);
			if (func_id != funcs::LEN) {
				if (consume(lexer::open_parenthesis)) {
					std::vector<const detail::node*> args = parse_func_call();
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
			const detail::node* n = parse_expression(get_precedence(lexer::op_subtraction, true));
			return nm.make_negation(n);
		}
		case lexer::open_parenthesis: {
			const detail::node* n = parse_expression(0);
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

const detail::node* parser::parse_infix(const detail::node* left, const lexer::token& infix) {
	auto& nm = current_context->node_manager();
	int p = get_precedence(infix.type);
	const lexer::token& prefix = m_tokens[m_index - 2];
	switch (infix.type) {
		case lexer::op_addition: {
			const detail::node* right = parse_expression(p);
			return nm.make_add({left, right});
		}
		case lexer::op_subtraction: {
			const detail::node* right = parse_expression(p);
			return nm.make_add({left, nm.make_negation(right)});
		}
		case lexer::op_multiplication: {
			const detail::node* right = parse_expression(p);
			return nm.make_mul({left, right});
		}
		case lexer::op_division: {
			const detail::node* right = parse_expression(p);
			return nm.make_div(left, right);
		}
		case lexer::op_power: {
			const detail::node* right = parse_expression(p - 1); // Right-associativity : a^b^c = a^(b^c)
			return nm.make_pow(left, right);
		}
		case lexer::op_modulo: {
			m_errors.emplace_back(unsupported, "Modulo is not yet supported.");
			return nullptr;
			const detail::node* right = parse_expression(p);
			return nm.make_mul({left, right});
		}
		case lexer::open_parenthesis: {
			auto func_id = detail::get_func_id(prefix.value);
			if (func_id != funcs::LEN) {
				std::vector<const detail::node*> args = parse_func_call();
				return nm.make_func(func_id, args);
			}
			const detail::node* right = parse_expression(0);
			advance();
			return nm.make_mul({left, right});
		}

		// Handle implicit multiplication
		case lexer::identifier: {
			auto func_id = detail::get_func_id(infix.value);
			if (func_id != funcs::LEN) {
				if (consume(lexer::open_parenthesis)) {
					std::vector<const detail::node*> args = parse_func_call();
					return nm.make_mul({left, nm.make_func(func_id, args)});
				}
				return nullptr;
			}
			const detail::node* right = nm.make_symbol(infix.value);
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

std::vector<const detail::node*> parser::parse_func_call() {
	std::vector<const detail::node*> args;
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

expression sym::parse(const lexer& lexer) {
	parser p(lexer);
	return p.parse(0);
}

expression sym::parse(const std::string& input) {
	lexer l;
	l.tokenize(input);
	parser p(l);
	return p.parse(0);
}
