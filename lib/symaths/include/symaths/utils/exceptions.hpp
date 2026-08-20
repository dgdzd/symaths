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

#ifndef SYM_EXCEPTIONS_HPP
#define SYM_EXCEPTIONS_HPP

#include "symaths/parsing/lexer.hpp"

#include <exception>

namespace sym {
	enum error_type {
		NONE = -1,
		unknown_identifier,
		unexpected_token,
		unsupported,
		type_error,
		invalid_signature,
	};

	class exception : public std::exception {
		std::string real_msg;
	protected:
		std::string m_message;

	public:
		exception() = default;
		exception(const std::string& msg);

		const char* what() const noexcept override;
		[[nodiscard]] std::string string() const;

		bool operator==(const exception& other) const;
	};

	class parsing_error : public exception {
		error_type m_err = NONE;
		lexer::token m_token;

	public:
		parsing_error() = default;
		parsing_error(error_type err, const lexer::token& token, const std::string& message);
	};
}

#endif
