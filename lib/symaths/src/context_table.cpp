#include "symaths/context_table.hpp"

using namespace sym;

void context_table_t::add_named_entry(const std::string& varname, detail::expression_value_t entry) {
	m_table[varname] = entry;
}

void context_table_t::remove_entry(const std::string& varname) {
	m_table.erase(varname);
}


void context_table_t::clear() {
	m_table.clear();
}

std::optional<expression> context_table_t::find_expression(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		const auto* ptr = std::get<const detail::mathexpr_node*>(result);
		if (!ptr) {
			m_table.erase(name);
			return std::nullopt;
		}
		return expression(ptr);
	}
	return std::nullopt;
}

std::optional<predicate> context_table_t::find_predicate(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		const auto* ptr = std::get<const detail::predicate_node*>(result);
		if (!ptr) {
			m_table.erase(name);
			return std::nullopt;
		}
		return predicate(ptr);
	}
	return std::nullopt;
}

std::optional<set> context_table_t::find_set(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		const auto* ptr = std::get<const detail::set_node*>(result);
		if (!ptr) {
			m_table.erase(name);
			return std::nullopt;
		}
		return set(ptr);
	}
	return std::nullopt;
}

std::optional<number> context_table_t::find_number(const std::string& name) {
	if (m_table.contains(name)) {
		return std::get<number>(m_table[name]);
	}
	return std::nullopt;
}

std::optional<bool> context_table_t::find_bool(const std::string& name) {
	if (m_table.contains(name)) {
		return std::get<bool>(m_table[name]);
	}
	return std::nullopt;
}