#include "symaths/detail/object.hpp"

#include "symaths/detail/mathexpr_node.hpp"
#include "symaths/detail/predicate_node.hpp"
#include "symaths/detail/set_node.hpp"

using namespace sym;

object::operator bool() const {
	return m_root.has_value();
}

bool object::operator!() const {
	return !m_root.has_value();
}

std::string object::string() const {
	if (!m_root.has_value()) return "";
	return std::visit(overloaded {
		[&](const bool& b) -> std::string { return b ? "true" : "false"; },
		[&](const number& n) -> std::string { return n.string(); },
		[&](const exception& e) -> std::string { return e.string(); },
		[&](const auto& v) -> std::string { return v->string(); }
	}, m_root.value());
}

detail::value_type_t object::type() const {
	if (!m_root) return detail::null;
	return static_cast<detail::value_type_t>(m_root.value().index());
}