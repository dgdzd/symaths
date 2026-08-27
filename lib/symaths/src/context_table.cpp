#include "symaths/context_table.hpp"

#include "symaths/builtin_functions.hpp"

using namespace sym;

context_table_t::context_table_t() {
	detail::init_builtin_functions(m_funcs);
}

std::ostream* context_table_t::out_stream() const {
	return m_out;
}

std::ostream*& context_table_t::out_stream() {
	return m_out;
}

void context_table_t::register_func(const detail::internal_func_descriptor& desc) {
	m_funcs.push_back(desc);
}

void context_table_t::add_entry(const std::string& varname, detail::expression_value_t entry) {
	m_table[varname] = {static_cast<detail::value_type_t>(entry.index()), object{entry}};
}

void context_table_t::add_uninitialized_entry(const std::string& varname, detail::value_type_t type) {
	if (!m_table.contains(varname)) m_table[varname] = {type, object{std::nullopt}};
}

void context_table_t::remove_entry(const std::string& varname) {
	m_table.erase(varname);
}

void context_table_t::clear() {
	m_table.clear();
}


uint32_t context_table_t::get_func_id(const std::string& name) const {
	auto& table = m_funcs;
	for (uint32_t i = 0; i < table.size(); ++i) {
		if (table[i].name == name) return i;
	}
	return UINT32_MAX;
}

uint32_t context_table_t::func_count() const {
	return m_funcs.size();
}

const detail::internal_func_descriptor& context_table_t::get_func(uint32_t id) const {
	return m_funcs[id];
}

std::vector<args_list_t> context_table_t::get_candidates(const detail::internal_func_descriptor& builtin, size_t arguments_count) const {
	std::vector<args_list_t> candidates;
	for (auto& args : builtin.arg_types) {
		if (args.size() == arguments_count) {
			candidates.push_back(args);
		}
	}
	return candidates;
}


std::optional<object> context_table_t::find(const std::string& name) {
	if (m_table.contains(name)) {
		return m_table[name].value;
	}
	return std::nullopt;
}


std::optional<bool> context_table_t::find_bool(const std::string& name) {
	if (m_table.contains(name)) {
		return m_table[name].value.cast<bool>();
	}
	return std::nullopt;
}

std::optional<number> context_table_t::find_number(const std::string& name) {
	if (m_table.contains(name)) {
		return m_table[name].value.cast<number>();
	}
	return std::nullopt;
}

std::optional<expression> context_table_t::find_expression(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		if (result.type != detail::mathexpr_) {
			m_table.erase(name);
			return std::nullopt;
		}
		return result.value.cast<expression>();
	}
	return std::nullopt;
}

std::optional<predicate> context_table_t::find_predicate(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		if (result.type != detail::predicate_) {
			m_table.erase(name);
			return std::nullopt;
		}
		return result.value.cast<predicate>();
	}
	return std::nullopt;
}

std::optional<set> context_table_t::find_set(const std::string& name) {
	if (m_table.contains(name)) {
		auto result = m_table[name];
		if (result.type != detail::set_) {
			m_table.erase(name);
			return std::nullopt;
		}
		return result.value.cast<set>();
	}
	return std::nullopt;
}