#include "symaths/set.hpp"

using namespace sym;

set::set(const detail::set_node* node) {
	root = node;
}
