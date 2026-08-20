#include "symaths/predicate.hpp"

#include "symaths/program.hpp"

using namespace sym;

predicate::predicate(const detail::predicate_node* node) {
	root = node;
}

predicate::predicate(const program_output& out) {
	root = out.cast<const detail::predicate_node*>();
}


bool predicate::operator()() const {
	return true;
}

std::string predicate::string() const {
	return "";
}
