#include "symaths/parsing/lexer.hpp"

#include <cctype>
#include <map>
#include <optional>

using namespace sym;

std::map<char, lexer::token_type> op_types = {
	{'+', lexer::token_type::op_addition},
	{'-', lexer::token_type::op_subtraction},
	{'*', lexer::token_type::op_multiplication},
	{'/', lexer::token_type::op_division},
	{'^', lexer::token_type::op_power},
	{'%', lexer::token_type::op_modulo},
};

bool isoperation(char c) {
	static const std::string operations = "+-*/^%";
	return operations.contains(c);
}

bool isoperation(lexer::token_type type) {
	return type == lexer::token_type::op_addition || type == lexer::token_type::op_subtraction
	|| type == lexer::token_type::op_multiplication || type == lexer::token_type::op_division
	|| type == lexer::token_type::op_power || type == lexer::token_type::op_modulo;
}

lexer::token next_token(char*& pc) {
	std::optional<lexer::token> current_token;

	char& c = *pc;
	while (c != '\0') {
		if (isalpha(c)) {
			if (!current_token) {
				// New number
				current_token = {lexer::identifier, std::string(1, c)};
			}
			else if (current_token.value().type != lexer::identifier) {
				return current_token.value();
			}
			else {
				current_token.value().value += c;
			}
		}

		else if (isdigit(c)) {
			if (!current_token) {
				// New number
				current_token = {lexer::number, std::string(1, c)};
			}
			else if (auto t = current_token.value().type; !(t == lexer::number || t == lexer::identifier)) {
				return current_token.value_or(lexer::token{lexer::NONE, ""});
			}
			else {
				current_token.value().value += c;
			}
		}

		else if (isspace(c)) {
			if (current_token) {
				pc++;
				return current_token.value();
			}
		}

		else if (isoperation(c)) {
			if (!current_token) {
				current_token = {op_types[c], std::string(1, c)};
			}
			else if (isoperation(current_token.value().type)) {
				return current_token.value_or(lexer::token{lexer::NONE, ""});
			}
			else {
				return current_token.value();
			}
		}

		else if (c == '(') {
			if (!current_token) {
				pc++;
				return {lexer::open_parenthesis, "("};
			}
			return current_token.value();
		}

		else if (c == ')') {
			if (!current_token) {
				pc++;
				return {lexer::close_parenthesis, ")"};
			}
			return current_token.value();
		}

		else if (c == ',') {
			if (!current_token) {
				pc++;
				return {lexer::comma, ","};
			}
			return current_token.value();
		}

		else {
			if (!current_token) {
				pc++;
				return {lexer::error, std::string(1, c)};
			}
			return current_token.value();
		}

		c = *++pc; // Get reference to next char
	}

	return current_token.value_or(lexer::token{lexer::NONE, ""});
}

void lexer::tokenize(std::string input) {
	m_tokens.clear();
	char* cursor = input.data();
	while (*cursor != '\0') {
		auto tok = next_token(cursor);
		if (tok.type != NONE) {
			m_tokens.push_back(tok);
		}
	}
	m_tokens.emplace_back(eof, "");
}

const std::vector<lexer::token>& lexer::tokens() const {
	return m_tokens;
}

std::vector<lexer::token>& lexer::tokens() {
	return m_tokens;
}

std::string sym::get_token_type_str(lexer::token_type type) {
	switch (type) {
		case lexer::token_type::NONE:
			return "NONE";
		case lexer::token_type::eof:
			return "EOF";
		case lexer::token_type::number:
			return "number";
		case lexer::token_type::identifier:
			return "identifier";
		case lexer::token_type::op_addition:
			return "op_addition";
		case lexer::token_type::op_subtraction:
			return "op_subtraction";
		case lexer::token_type::op_multiplication:
			return "op_multiplication";
		case lexer::token_type::op_division:
			return "op_division";
		case lexer::token_type::op_power:
			return "op_power";
		case lexer::token_type::op_modulo:
			return "op_modulo";
		case lexer::token_type::open_parenthesis:
			return "open_parenthesis";
		case lexer::token_type::close_parenthesis:
			return "close_parenthesis";
		case lexer::token_type::comma:
			return "comma";
		case lexer::token_type::error:
			return "<error>";
		default:
			return "<unknown>";
	}
}
