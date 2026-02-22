#ifndef LEXER_HPP
#define LEXER_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace sym {
	class lexer {
	public:
		enum token_type : uint8_t {
			NONE = -1,
			eof,
			number,
			identifier,
			operation,
			open_parenthesis,
			close_parenthesis,
			comma,
		};
		struct token {
			token_type type = NONE;
			std::string value;
		};

	private:
		std::vector<token> m_tokens;

	public:
		lexer() = default;

		void tokenize(std::string input);
	};
}

#endif
