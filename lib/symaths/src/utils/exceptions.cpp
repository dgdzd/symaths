#include "symaths/utils/exceptions.hpp"

#include <format>

using namespace sym;

exception::exception(const std::string& msg) {
	m_message = msg;
}

const char* exception::what() const noexcept {
	return m_message.c_str();
}

std::string exception::string() const {
	return m_message;
}

bool exception::operator==(const exception& other) const {
	return m_message == other.m_message;
}


parsing_error::parsing_error(error_type err, const lexer::token& token, const std::string& message) {
	m_err = err;
	m_token = token;
	m_message = std::format("[{}:{}] {}", m_token.line, m_token.column, message);
}