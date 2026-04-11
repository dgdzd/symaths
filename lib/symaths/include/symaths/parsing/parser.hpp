/*
 *	                            _   _
 *	  ___ _   _ _ __ ___   __ _| |_| |__  ___
 *	 / __| | | | '_ ` _ \ / _` | __| '_ \/ __|   Symbolic maths for C++
 *	 \__ \ |_| | | | | | | (_| | |_| | | \__ \   Version : 0.0.1
 *	 |___/\__, |_| |_| |_|\__,_|\__|_| |_|___/   https://github.com/dgdzd/symaths
 *		  |___/
 *
 * All source code is distributed under the GNU General Public License v2.0.
 *
 */

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
		enum error_type {
			NONE = -1,
			unknown_identifier,
			unexpected_token,
			unsupported,
		};
		struct error {
			error_type type;
			std::string desc;
		};

		parser() = default;
		explicit parser(const lexer& lexer) : m_tokens(lexer.m_tokens) {}
		explicit parser(lexer&& lexer) : m_tokens(std::move(lexer.m_tokens)) {}

		[[nodiscard]] bool has_tokens() const;
		[[nodiscard]] const lexer::token& current_token() const;
		const lexer::token& advance();

		const detail::node* parse(int precedence_limit);

	private:
		std::vector<lexer::token> m_tokens;
		std::vector<error> m_errors;
		size_t m_index = 0;

		const detail::node* parse_expression(int precedence_limit);
		const detail::node* parse_prefix(const lexer::token& prefix);
		const detail::node* parse_infix(const detail::node* left, const lexer::token& infix);
		std::vector<const detail::node*> parse_func_call();
		bool consume(lexer::token_type type);
		bool expect_next(lexer::token_type type);
	};

	expression parse(const lexer& lexer);
	expression parse(const std::string& input);
}

#endif
