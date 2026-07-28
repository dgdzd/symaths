#include "symaths/predicate.hpp"

using namespace sym;

predicate::predicate(const detail::predicate_node* node) {
	root = node;
}

bool predicate::operator()() const {
	return true;
}

std::string predicate::string() const {
	return "";
}
