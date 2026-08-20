#include "symaths/detail/predicate_node.hpp"

using namespace sym::detail;

bool predicate_node::is_ground() const {
	return false; // TODO
}

std::string predicate_node::string() const {
	return "<predicate object>"; // TODO
}