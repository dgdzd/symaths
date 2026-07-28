#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "symaths/expression.hpp"
#include "symaths/set.hpp"

#include <map>

namespace sym {
	class function {
		std::string name;
		std::map<set, expression> parts;

		function(const expression& part);
	};
}

#endif
