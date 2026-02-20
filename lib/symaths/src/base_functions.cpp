#include "symaths/base_functions.hpp"

#include "symaths/symaths.hpp"
#include "symaths/detail/nodes.hpp"

#include <stdexcept>

using namespace sym;

static detail::builtin_func_descriptor builtin_table[funcs::LEN];

double cos_eval(const std::vector<const detail::node*>& args);
const detail::node* cos_reduce(const std::vector<const detail::node*>& args);
const detail::node* cos_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double sin_eval(const std::vector<const detail::node*>& args);
const detail::node* sin_reduce(const std::vector<const detail::node*>& args);
const detail::node* sin_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double tan_eval(const std::vector<const detail::node*>& args);
const detail::node* tan_reduce(const std::vector<const detail::node*>& args);
const detail::node* tan_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double acos_eval(const std::vector<const detail::node*>& args);
const detail::node* acos_reduce(const std::vector<const detail::node*>& args);
const detail::node* acos_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double asin_eval(const std::vector<const detail::node*>& args);
const detail::node* asin_reduce(const std::vector<const detail::node*>& args);
const detail::node* asin_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double atan_eval(const std::vector<const detail::node*>& args);
const detail::node* atan_reduce(const std::vector<const detail::node*>& args);
const detail::node* atan_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double exp_eval(const std::vector<const detail::node*>& args);
const detail::node* exp_reduce(const std::vector<const detail::node*>& args);
const detail::node* exp_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double ln_eval(const std::vector<const detail::node*>& args);
const detail::node* ln_reduce(const std::vector<const detail::node*>& args);
const detail::node* ln_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double log10_eval(const std::vector<const detail::node*>& args);
const detail::node* log10_reduce(const std::vector<const detail::node*>& args);
const detail::node* log10_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double cosh_eval(const std::vector<const detail::node*>& args);
const detail::node* cosh_reduce(const std::vector<const detail::node*>& args);
const detail::node* cosh_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double sinh_eval(const std::vector<const detail::node*>& args);
const detail::node* sinh_reduce(const std::vector<const detail::node*>& args);
const detail::node* sinh_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double tanh_eval(const std::vector<const detail::node*>& args);
const detail::node* tanh_reduce(const std::vector<const detail::node*>& args);
const detail::node* tanh_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double sqrt_eval(const std::vector<const detail::node*>& args);
const detail::node* sqrt_reduce(const std::vector<const detail::node*>& args);
const detail::node* sqrt_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);

double abs_eval(const std::vector<const detail::node*>& args);
const detail::node* abs_reduce(const std::vector<const detail::node*>& args);
const detail::node* abs_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt);


void detail::init_builtin_functions() {
	using namespace funcs;

	builtin_table[static_cast<size_t>(funcs::cos)] = {
		"cos", cos_eval, cos_reduce, cos_derivative
	};
	builtin_table[static_cast<size_t>(funcs::sin)] = {
		"sin", sin_eval, sin_reduce, sin_derivative
	};
	builtin_table[static_cast<size_t>(funcs::tan)] = {
		"tan", tan_eval, tan_reduce, tan_derivative
	};
	builtin_table[static_cast<size_t>(funcs::acos)] = {
		"acos", acos_eval, acos_reduce, acos_derivative
	};
	builtin_table[static_cast<size_t>(funcs::asin)] = {
		"asin", asin_eval, asin_reduce, asin_derivative
	};
	builtin_table[static_cast<size_t>(funcs::atan)] = {
		"atan", atan_eval, atan_reduce, atan_derivative
	};
	builtin_table[static_cast<size_t>(funcs::exp)] = {
		"exp", exp_eval, exp_reduce, exp_derivative
	};
	builtin_table[static_cast<size_t>(funcs::ln)] = {
		"ln", ln_eval, ln_reduce, ln_derivative
	};
	builtin_table[static_cast<size_t>(funcs::log10)] = {
		"log10", log10_eval, log10_reduce, log10_derivative
	};
	builtin_table[static_cast<size_t>(funcs::cosh)] = {
		"cosh", cosh_eval, cosh_reduce, cosh_derivative
	};
	builtin_table[static_cast<size_t>(funcs::sinh)] = {
		"sinh", sinh_eval, sinh_reduce, sinh_derivative
	};
	builtin_table[static_cast<size_t>(funcs::tanh)] = {
		"tanh", tanh_eval, tanh_reduce, tanh_derivative
	};
	builtin_table[static_cast<size_t>(funcs::sqrt)] = {
		"sqrt", sqrt_eval, sqrt_reduce, sqrt_derivative
	};
	builtin_table[static_cast<size_t>(funcs::abs)] = {
		"abs", abs_eval, abs_reduce, abs_derivative
	};
}

const detail::builtin_func_descriptor& detail::get_func(funcs::builtin_fn_id id) {
	return builtin_table[static_cast<size_t>(id)];
}


double cos_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: cos only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::cos(value);
}

const detail::node* cos_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: cos only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::cos, args);
}

const detail::node* cos_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// cos'(u) = u'(-sin(u))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: cos only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_negation(nm.make_func(funcs::sin, args))
	});
}

double sin_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sin only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::sin(value);
}

const detail::node* sin_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sin only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::sin, args);
}

const detail::node* sin_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// sin'(u) = u'cos(u)
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sin only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_func(funcs::cos, args)
	});
}

double tan_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: tan only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::tan(value);
}

const detail::node* tan_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: tan only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::tan, args);
}

const detail::node* tan_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// tan'(u) = u'(1 + tan²(u))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: tan only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_add({
			nm.make_constant(1),
			nm.make_pow(nm.make_func(funcs::tan, args), nm.make_constant(2))
		})
	});
}

double acos_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: acos only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::acos(value);
}

const detail::node* acos_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: acos only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::acos, args);
}

const detail::node* acos_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// acos'(u) = -u' / sqrt(1 - u²)
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: acos only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_negation(
		nm.make_div(
			df,
			nm.make_func(funcs::sqrt, {nm.make_add({
				nm.make_constant(1),
				nm.make_negation(nm.make_pow(args[0], nm.make_constant(2)))
			})})
		)
	);
}

double asin_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: asin only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::asin(value);
}

const detail::node* asin_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: asin only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::asin, args);
}

const detail::node* asin_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// asin'(u) = u' / sqrt(1 - u²)
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: asin only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_div(
		df,
		nm.make_func(funcs::sqrt, {nm.make_add({
			nm.make_constant(1),
			nm.make_negation(nm.make_pow(args[0], nm.make_constant(2)))
		})})
	);
}

double atan_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: atan only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::atan(value);
}

const detail::node* atan_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: atan only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::atan, args);
}

const detail::node* atan_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// atan'(u) = u' / (1 + u²)
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: atan only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_div(
		df,
		nm.make_add({
			nm.make_constant(1),
			nm.make_pow(args[0], nm.make_constant(2))
		})
	);
}

double exp_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: exp only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::exp(value);
}

const detail::node* exp_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: exp only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::exp, args);
}

const detail::node* exp_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// exp'(u(x)) = u'(x) exp(u(x))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: exp only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_func(funcs::exp, args)
	});
}

double ln_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: ln only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::log(value);
}

const detail::node* ln_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: ln only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::ln, args);
}

const detail::node* ln_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// ln'(u(x)) = u'(x)/u(x)
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: ln only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_div(
		df,
		args[0]
	);
}

double log10_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: log10 only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::log10(value);
}

const detail::node* log10_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: log10 only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::log10, args);
}

const detail::node* log10_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// log10'(u(x)) = u'(x)/(ln(10) u(x))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: log10 only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_div(
		df,
		nm.make_mul({nm.make_func(funcs::ln, {nm.make_constant(10)}), args[0]})
	);
}

double cosh_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: cosh only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::cosh(value);
}

const detail::node* cosh_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: cosh only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::cosh, args);
}

const detail::node* cosh_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// cosh'(x) = u'(x)sinh(u(x))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: cosh only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_func(funcs::sinh, args)
	});
}

double sinh_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sinh only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::sinh(value);
}

const detail::node* sinh_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sinh only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::sinh, args);
}

const detail::node* sinh_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// sinh'(x) = u'(x)cosh(u(x))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sinh only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_func(funcs::cosh, args)
	});
}

double tanh_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: tanh only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::tanh(value);
}

const detail::node* tanh_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: tanh only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::tanh, args);
}

const detail::node* tanh_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// tanh'(x) = u'(x)(1 - tanh²(u(x))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: tanh only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_mul({
		df,
		nm.make_add({
			nm.make_constant(1),
			nm.make_negation(nm.make_pow(nm.make_func(funcs::tanh, args), nm.make_constant(2)))
		})
	});
}

double sqrt_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sqrt only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::sqrt(value);
}

const detail::node* sqrt_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sqrt only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::sqrt, args);
}

const detail::node* sqrt_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// sqrt'(u(x)) = u'(x)/(2 sqrt(u(x)))
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: sqrt only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	auto& nm = current_context->node_manager();
	return nm.make_div(
		df,
		nm.make_mul({
			nm.make_constant(2),
			nm.make_func(funcs::sqrt, args)
		})
	);
}

double abs_eval(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: abs only supports 1 argument");
	}
	double value = args[0]->eval(nullptr);
	return std::abs(value);
}

const detail::node* abs_reduce(const std::vector<const detail::node*>& args) {
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: abs only supports 1 argument");
	}
	return current_context->node_manager().make_func(funcs::abs, args);
}

const detail::node* abs_derivative(const std::vector<const detail::node*>& args, const detail::node* wrt) {
	// abs'(u(x)) = u(x) < 0 ? -u'(x) : u'(x)
	// TODO : multi-parts functions
	if (args.size() > 1) {
		throw std::invalid_argument("funcs:builtin: abs only supports 1 argument");
	}
	auto df = differentiate(args[0], wrt).root;
	return df;
}