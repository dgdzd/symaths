#ifndef PARSER_HPP
#define PARSER_HPP

#include "symaths/expression.hpp"
#include "symaths/parsing/lexer.hpp"

namespace sym {
	namespace detail {
		class node;
	}

	class parser {
	public:

		static expression parse(const std::string& input);
		static expression parse(const lexer& lexer);
	};
}

#endif
