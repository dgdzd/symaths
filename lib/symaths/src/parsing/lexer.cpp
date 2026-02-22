#include "../../include/symaths/parsing/lexer.hpp"

#include <cctype>
#include <optional>

using namespace sym;

bool isoperation(char c) {
	static const std::string operations = "+-*/^%";
	return operations.contains(c);
}

lexer::token next_token(char*& pc) {
	std::optional<lexer::token> current_token;

	char& c = *pc;
	while (c != '\0') {
		if (isdigit(c)) {
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

		else if (isalpha(c)) {
			if (!current_token) {
				// New number
				current_token = {lexer::identifier, std::string(1, c)};
			}
			else if (current_token.value().type != lexer::identifier) {
				return current_token.value_or(lexer::token{lexer::NONE, ""});
			}
			else {
				current_token.value().value += c;
			}
		}

		else if (isspace(c)) {
			pc++;
			return current_token.value_or(lexer::token{lexer::NONE, ""});
		}

		else if (isoperation(c)) {
			if (!current_token) {
				current_token = {lexer::operation, std::string(1, c)};
			}
			else if (current_token.value().type != lexer::operation) {
				return current_token.value_or(lexer::token{lexer::NONE, ""});
			}
			else {
				current_token.value().value += c;
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
}
