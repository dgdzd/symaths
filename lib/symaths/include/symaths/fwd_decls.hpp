#ifndef SYM_FWD_DECLS_HPP
#define SYM_FWD_DECLS_HPP

#include <variant>

namespace sym {
	namespace objs {
		class variable;
		class constant;
		class addition;
		class multiplication;
		class division;
		class power;
	}

	using real_t = double;
	using natural_t = unsigned int;
	using integer_t = int;
	using value_t = std::variant<real_t, natural_t, integer_t>;
}

#endif
