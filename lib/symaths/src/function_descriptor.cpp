#include "symaths/function_descriptor.hpp"

using namespace sym;

template<detail::is_object T>
detail::value_type_t function_descriptor<T>::return_type() const {
	return static_cast<detail::value_type_t>(variant_index<detail::expression_value_t, T>());
}

template<detail::is_object T>
function_descriptor<T>& function_descriptor<T>::with_name(const std::string& name) {
	m_name = name;
	return *this;
}

template<detail::is_object T>
function_descriptor<T>& function_descriptor<T>::with_handler(const func_handler_t<T>& handler) {
	m_handler = handler;
	return *this;
}

template<detail::is_object T>
function_descriptor<T>& function_descriptor<T>::add_signature(const args_list_t& args) {
	m_args.push_back(args);
	return *this;
}