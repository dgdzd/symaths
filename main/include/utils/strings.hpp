#ifndef SYM_STRINGS_HPP
#define SYM_STRINGS_HPP

#include <string>
#include <vector>

namespace symtool::utils {
	extern std::string trim_trailing(std::string_view str, char c = ' ', int limit = -1);
	extern std::string trim_leading(std::string_view str, char c = ' ', int limit = -1);
	extern std::string trim(std::string_view str, char c = ' ');
	extern std::vector<std::string> split(std::string_view str, char c = ' ', int limit = 0);
	extern std::vector<std::string> splitm(std::string_view str, std::string_view sep, int limit = -1);
	extern std::vector<std::u8string> split(std::u8string_view str, char8_t c = ' ', int limit = 0);
}

#endif
