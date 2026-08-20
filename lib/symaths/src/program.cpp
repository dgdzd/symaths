#include "symaths/program.hpp"

#include "symaths/symaths.hpp"

using namespace sym;

std::string program_output::string() {
	if (!m_root.has_value()) return "";
	return std::visit(overloaded {
		[&](const bool& b) -> std::string { return b ? "true" : "false"; },
		[&](const number& n) -> std::string { return n.string(); },
		[&](const exception& e) -> std::string { return e.string(); },
		[&](const auto& v) -> std::string { return v->string(); }
	}, m_root.value());
}


program::program(const detail::statement_node* node) {
	root = node;
}

program::program(const std::vector<const detail::statement_node*>& statements) {
	root = make_sequence(statements);
}

program_output program::execute() const {
	if (!root) return program_output{};
	context_table_t ctx;
	return program_output{root->eval(&ctx)};
}

program_output program::evaluate(context_table_t* ctx) const {
	if (!root) return program_output{};
	if (!ctx) ctx = &current_context->context_table();
	return program_output{root->eval(ctx)};
}
