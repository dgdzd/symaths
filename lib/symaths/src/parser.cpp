#include "symaths/parser.hpp"

#include <cctype>
#include <optional>

using namespace sym;

void parser::lex(const std::string& input) {
	std::optional<token> current_token;
	for (char c : input) {
		if (isdigit(c)) {
			if (!current_token) {
				// New number
				current_token = {atom, std::string(1, c)};
			}
			else if (auto t = current_token.value().type; t != atom) {
				current_token.value().value += c;
				m_tokens.push_back(current_token.value());
				current_token = {atom, std::string(1, c)};
			}
			else {
				current_token.value().value += c;
			}
		}
		else if (isalpha(c)) {
			if (!current_token) {
				// New number
				current_token = {atom, std::string(1, c)};
			}
			else if (auto t = current_token.value().type; t != atom) {
				m_tokens.push_back(current_token.value());
				current_token = {atom, std::string(1, c)};
			}
			else if (isdigit(current_token.value().value[0])) {
				m_tokens.push_back(current_token.value());
				current_token = {atom, std::string(1, c)};
			}
			else {
				current_token.value().value += c;
			}
		}
		else if (isspace(c)) {
			if (!current_token || current_token.value().type == operation) {
				continue;
			}

			if (isalpha(current_token.value().value[0])) {

			}
		}
	}
}
