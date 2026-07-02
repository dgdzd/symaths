#include "symaths/context_table.hpp"

using namespace sym;

void context_table::add_entry(const detail::expression_node* obj) {
	m_objects.emplace_back(expression, obj);
}

void context_table::add_entry(const detail::predicate_node* obj) {
	m_objects.emplace_back(predicate, obj);
}

void context_table::add_entry(const detail::set_node* obj) {
	m_objects.emplace_back(set, obj);
}

void context_table::remove_entry(const void* obj) {
	for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
		if ((*it).value == obj) {
			m_objects.erase(it);
		}
	}
}

void context_table::clear() {
	m_objects.clear();
}

const std::vector<context_table::entry>& context_table::get_objects() const {
	return m_objects;
}

std::vector<sym::expression> context_table::get_expressions() const {
	// TODO
	return {};
}

std::vector<detail::predicate_node> context_table::get_predicates() const {
	// TODO
	return {};
}

std::vector<detail::set_node> context_table::get_sets() const {
	// TODO
	return {};
}

std::optional<detail::predicate_node> context_table::find_predicate(const std::string& name) const {
	// TODO
	return {};
}

std::optional<detail::set_node> context_table::find_set(const std::string& name) const {
	// TODO
	return {};
}
