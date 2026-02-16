#ifndef SYM_BASE_FUNCTIONS_HPP
#define SYM_BASE_FUNCTIONS_HPP

#include <vector>

namespace sym {
	namespace detail {
		class node;
	}

	namespace funcs {
		enum builtin_fn_id : uint32_t {
			cos, sin, tan, acos, asin, atan,
			exp, ln, log10, cosh, sinh, tanh,
			sqrt,
			abs,
			LEN
		};
	}

	namespace detail {
		void init_builtin_functions();

		struct builtin_func_descriptor {
			using eval_t = double (*)(const std::vector<const node*>& args);
			using reduce_t = const node* (*)(const std::vector<const node*>& args);
			using derivative_t = const node* (*)(const std::vector<const node*>& args, const node* wrt);

			const char* name;
			eval_t eval;
			reduce_t reduce;
			derivative_t derivative;
		};

		const builtin_func_descriptor& get_func(funcs::builtin_fn_id id);
	}
}

#endif
