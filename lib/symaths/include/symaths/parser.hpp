#ifndef PARSER_HPP
#define PARSER_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace sym {
	class parser {
	public:
		enum token_type : uint8_t {
			atom, operation, eof
		};
		struct token {
			token_type type;
			std::string value;
		};
	private:
		std::vector<token> m_tokens;
	public:
		parser() = default;

		void lex(const std::string& input);
	};
}

#endif
