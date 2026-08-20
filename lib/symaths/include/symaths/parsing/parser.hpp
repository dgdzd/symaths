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

#ifndef SYM_PARSER_HPP
#define SYM_PARSER_HPP

#include "symaths/context_table.hpp"
#include "symaths/expression.hpp"
#include "symaths/program.hpp"
#include "symaths/parsing/lexer.hpp"
#include "symaths/utils/exceptions.hpp"

#include <optional>

namespace sym {
	namespace detail {
		class mathexpr_node;
	}

	class parser {
	public:
		struct error {
			error_type type;
			std::string desc;
		};

		parser() = default;
		explicit parser(const lexer& lexer) : m_tokens(lexer.m_tokens) {}
		explicit parser(lexer&& lexer) : m_tokens(std::move(lexer.m_tokens)) {}

		[[nodiscard]] bool has_errors() const;
		[[nodiscard]] exception build_error() const;

		[[nodiscard]] bool has_tokens() const;
		[[nodiscard]] const lexer::token& current_token() const;
		const lexer::token& advance();

		const detail::statement_node* parse();
		const detail::statement_node* parse_stmt(context_table_t* table, detail::value_type_t ctx_type);

	private:
		size_t m_index = 0;
		std::vector<lexer::token> m_tokens;
		std::vector<parsing_error> m_errors;
		std::map<std::string, detail::value_type_t> m_types;
		std::vector<detail::value_type_t> m_context_type;

		std::optional<bool> parse_bool();
		std::optional<number> parse_number();
		const detail::mathexpr_node* parse_mathexpr(int precedence_limit);
		const detail::predicate_node* parse_predicate(int precedence_limit);

		std::string parse_name();
		const detail::mathexpr_node* parse_prefix(const lexer::token& prefix);
		const detail::mathexpr_node* parse_infix(const detail::mathexpr_node* left, const lexer::token& infix);
		std::vector<const detail::mathexpr_node*> parse_func_call();
		const detail::mathexpr_node* parse_builtin_call(context_table_t* ctx, uint32_t builtin_id);
		bool consume(lexer::token_type type);
		bool expect(lexer::token_type type);
		bool expect2(lexer::token_type type1, lexer::token_type type2);
		bool expect_next(lexer::token_type type);
		std::optional<lexer::token> expect_identifier(const std::string& name);
	};

	program parse_single(const lexer& lexer);
	program parse_single(const std::string& input);

	program parse(const lexer& lexer);
	program parse(const std::string& input);

	expression parse_expression(const lexer& lexer);
	expression parse_expression(const std::string& input);
}

#endif
