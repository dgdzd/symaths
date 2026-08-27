#ifndef SYM_BUILTIN_FUNCTIONS_HPP
#define SYM_BUILTIN_FUNCTIONS_HPP

#include "symaths/context_table.hpp"
#include "symaths/detail/mathexpr_node.hpp"
#include "symaths/detail/statement_node.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace sym::detail {
	void init_builtin_functions(std::vector<internal_func_descriptor>& out);
}


#endif
