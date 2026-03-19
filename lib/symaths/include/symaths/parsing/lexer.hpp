#ifndef LEXER_HPP
#define LEXER_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace sym {
	class lexer {
		friend class parser;
	public:
		enum token_type : int8_t {
			NONE = -1,
			error,
			eof,

			number,
			identifier,

			op_addition,
			op_subtraction,
			op_multiplication,
			op_division,
			op_power,
			op_modulo,

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
		[[nodiscard]] const std::vector<token>& tokens() const;
		[[nodiscard]] std::vector<token>& tokens();
	};

	std::string get_token_type_str(lexer::token_type type);
}

bool isoperation(sym::lexer::token_type type);

#endif
