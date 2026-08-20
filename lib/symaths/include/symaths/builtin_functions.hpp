#ifndef SYM_BUILTIN_FUNCTIONS_HPP
#define SYM_BUILTIN_FUNCTIONS_HPP

#include "symaths/context_table.hpp"
#include "symaths/detail/mathexpr_node.hpp"
#include "symaths/detail/statement_node.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace sym {
	using builtin_handler_t = std::function<detail::expression_value_t(
		const std::vector<detail::expression_value_t>&,
		context_table_t*
	)>;
	using args_list_t = std::vector<detail::value_type_t>;

	struct builtin_descriptor {
		const char* name;
		builtin_handler_t handler;
		std::vector<args_list_t> arg_types;
	};

	uint32_t get_builtin_id(const std::string& name);
	uint32_t builtin_count();
	const builtin_descriptor& get_builtin(uint32_t id);
	std::vector<args_list_t> get_candidates(const builtin_descriptor& builtin, size_t arguments_count);
}

#endif
