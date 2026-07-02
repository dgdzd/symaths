#include "symaths/parsing/lexer.hpp"

#include <cctype>
#include <map>
#include <optional>

using namespace sym;

std::map<std::string, lexer::token_type> op_types = {
	{"+", lexer::token_type::op_addition},
	{"-", lexer::token_type::op_subtraction},
	{"*", lexer::token_type::op_multiplication},
	{"/", lexer::token_type::op_division},
	{"^", lexer::token_type::op_power},
	{"%", lexer::token_type::op_modulo},
	{"!", lexer::token_type::op_modulo},
	{"(", lexer::token_type::open_parenthesis},
	{")", lexer::token_type::close_parenthesis},
	{"{", lexer::token_type::open_crlbracket},
	{"}", lexer::token_type::close_crlbracket},
	{"[", lexer::token_type::open_sqbracket},
	{"]", lexer::token_type::close_sqbracket},
	{"|", lexer::token_type::vertical_bar},
	{",", lexer::token_type::comma},
	{":", lexer::token_type::colon},
	{";", lexer::token_type::semicolon},
	{"\"", lexer::token_type::quote},
	{"->", lexer::token_type::right_arrow},
};

bool isoperation(char c) {
	static const std::string operations = "+-*/^%!(){}[]|,:;\"=<>≤≥";
	return operations.contains(c);
}

lexer::token next_token(char*& pc) {
	std::optional<lexer::token> current_token;

	char& c = *pc;
	while (c != '\0') {
		bool ret = false;
		if (isalpha(c)) {
			if (!current_token) {
				// New number
				current_token = {lexer::identifier, std::string(1, c)};
			}
			else if (current_token.value().type != lexer::identifier) {
				ret = true;
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
				ret = true;
			}
			else {
				current_token.value().value += c;
			}
		}

		else if (isspace(c) || c == '\n') {
			if (current_token) {
				pc++;
				ret = true;
			}
		}

		else if (isoperation(c)) {
			if (!current_token) {
				auto sc = std::string(1, c);
				current_token = {op_types[sc], sc};
			}
			else if (auto& tok = current_token.value(); tok.type == lexer::number || tok.type == lexer::identifier) {
				return tok;
			}
			else {
				tok.value += c;
				if (op_types.contains(tok.value)) {
					tok.type = op_types[tok.value];
				}
				else {
					tok.type = lexer::NONE;
				}
			}
		}

		if (ret) {
			lexer::token& tok = current_token.value();
			if (tok.type == lexer::NONE) {
				while (tok.type == lexer::NONE && !tok.value.empty()) {
					tok.value.pop_back();
					pc--;
					if (op_types.contains(tok.value)) tok.type = op_types[tok.value];
				}
			}
			return tok;
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
		case lexer::token_type::op_factorial:
			return "op_factorial";
		case lexer::token_type::open_parenthesis:
			return "open_parenthesis";
		case lexer::token_type::close_parenthesis:
			return "close_parenthesis";
		case lexer::token_type::open_crlbracket:
			return "open_crlbracket";
		case lexer::token_type::close_crlbracket:
			return "close_crlbracket";
		case lexer::token_type::open_sqbracket:
			return "open_sqbracket";
		case lexer::token_type::close_sqbracket:
			return "close_sqbracket";
		case lexer::token_type::vertical_bar:
			return "vertical_bar";
		case lexer::token_type::comma:
			return "comma";
		case lexer::token_type::colon:
			return "colon";
		case lexer::token_type::semicolon:
			return "semicolon";
		case lexer::token_type::quote:
			return "quote";
		case lexer::token_type::error:
			return "<error>";
		default:
			return "<unknown>";
	}
}
