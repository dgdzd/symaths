#include "symaths/context_table.hpp"

using namespace sym;

void context_table_t::add_entry(const detail::expression_node* obj) {
	m_objects.emplace_back(std::make_shared<entry>(expression, obj));
}

void context_table_t::add_entry(const detail::predicate_node* obj) {
	m_objects.emplace_back(std::make_shared<entry>(predicate, obj));
}

void context_table_t::add_entry(const detail::set_node* obj) {
	m_objects.push_back(std::make_shared<entry>(set, obj));
}

void context_table_t::add_entry(std::shared_ptr<entry> p_entry) {
	m_objects.push_back(p_entry);
}

void context_table_t::add_named_entry(const std::string& name, std::shared_ptr<entry> p_entry) {
	m_objects.push_back(p_entry);
	m_table[name] = std::weak_ptr(p_entry);
}


void context_table_t::remove_entry(const void* obj) {
	for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
		if ((*it)->value == obj) {
			m_objects.erase(it);
		}
	}
}

void context_table_t::clear() {
	m_objects.clear();
}

const std::vector<std::shared_ptr<context_table_t::entry>>& context_table_t::get_objects() const {
	return m_objects;
}

std::vector<expression> context_table_t::get_expressions() const {
	std::vector<sym::expression> results;
	for (auto& entry : m_objects) {
		if (entry->type == expression) {
			results.emplace_back(static_cast<const detail::expression_node*>(entry->value));
		}
	}
	return results;
}

std::vector<predicate> context_table_t::get_predicates() const {
	std::vector<sym::predicate> results;
	for (auto& entry : m_objects) {
		if (entry->type == predicate) {
			results.emplace_back(static_cast<const detail::predicate_node*>(entry->value));
		}
	}
	return results;
}

std::vector<set> context_table_t::get_sets() const {
	std::vector<sym::set> results;
	for (auto& entry : m_objects) {
		if (entry->type == set) {
			results.emplace_back(static_cast<const detail::set_node*>(entry->value));
		}
	}
	return results;
}

std::optional<expression> context_table_t::find_expression(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		if (result.expired()) {
			m_table.erase(name);
			return std::nullopt;
		}
		auto ptr = result.lock();
		if (ptr->type != expression) {
			return std::nullopt;
		}
		return sym::expression(static_cast<const detail::expression_node*>(ptr->value));
	}
	return std::nullopt;
}

std::optional<predicate> context_table_t::find_predicate(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		if (result.expired()) {
			m_table.erase(name);
			return std::nullopt;
		}
		auto ptr = result.lock();
		if (ptr->type != predicate) {
			return std::nullopt;
		}
		return sym::predicate(static_cast<const detail::predicate_node*>(ptr->value));
	}
	return std::nullopt;
}

std::optional<set> context_table_t::find_set(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		if (result.expired()) {
			m_table.erase(name);
			return std::nullopt;
		}
		auto ptr = result.lock();
		if (ptr->type != set) {
			return std::nullopt;
		}
		return sym::set(static_cast<const detail::set_node*>(ptr->value));
	}
	return std::nullopt;
}
