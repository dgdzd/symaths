#ifndef COMPILE_HPP
#define COMPILE_HPP

#include <vector>

namespace sym {
	namespace detail {
		enum opcode : uint8_t {
			push_cst, push_var, call_fun,
			neg, add, sub, mul, div, pow,
		};

		struct instruction {
			opcode op;
			union {
				double val;
				size_t var_id;
				double (*fn)(const double*);
			};
		};
	}

	class compiled_expression {

	};
}

#endif
