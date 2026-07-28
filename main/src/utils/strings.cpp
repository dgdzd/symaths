#include "utils/strings.hpp"

using namespace symtool::utils;

std::string symtool::utils::trim_trailing(std::string_view str, char c, int limit) {
	if (str.empty() || limit == 0) {
		return str.data();
	}
	if (str[0] == c) {
		return trim_trailing(str.substr(1), c, limit-1);
	}

	return {str.data(), str.size()};
}

std::string symtool::utils::trim_leading(std::string_view str, char c, int limit) {
	if (str.empty() || limit == 0) {
		return str.data();
	}
	if (str.back() == c) {
		return trim_leading(str.substr(0, str.size() - 1), c, limit-1);
	}

	return {str.data(), str.size()};
}

std::string symtool::utils::trim(std::string_view str, const char c) {
	return trim_leading(trim_trailing(str, c), c);
}

std::vector<std::string> symtool::utils::split(std::string_view str, char c, int limit) {
	std::vector<std::string> parts;
	std::string part;
	int count = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == c && (limit == 0 || count < limit)) {
			parts.push_back(part);
			part = "";
			count++;
		}
		else {
			part += str[i];
		}
	}
	parts.push_back(part);
	return parts;
}

std::vector<std::string> symtool::utils::splitm(std::string_view str, std::string_view sep, int limit) {
	std::vector<std::string> parts;
	std::string_view remaining = str;

	while (!remaining.empty() && limit != 0) {
		auto pos = remaining.find(sep);
		if (pos == std::string::npos) {
			break;
		}
		parts.emplace_back(remaining.substr(0, pos).data());
		remaining = remaining.substr(pos + sep.size());
		limit--;
	}
	parts.emplace_back(remaining.data());

	return parts;
}


std::vector<std::u8string> symtool::utils::split(std::u8string_view str, char8_t c, int limit) {
	std::vector<std::u8string> parts;
	std::u8string part;
	int count = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == c && (limit == 0 || count < limit)) {
			parts.push_back(part);
			part = u8"";
			count++;
		}
		else {
			part += str[i];
		}
	}
	parts.push_back(part);
	return parts;
}