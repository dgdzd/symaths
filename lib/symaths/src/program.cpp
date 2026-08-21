#include "symaths/program.hpp"

#include "symaths/symaths.hpp"

using namespace sym;


program::program(const detail::statement_node* node, std::ostream* p_stream) {
	root = node;
	out_stream = p_stream;
}

program::program(const std::vector<const detail::statement_node*>& statements, std::ostream* p_stream) {
	root = make_sequence(statements);
	out_stream = p_stream;
}

context_table_t program::execute() const {
	if (!root) return context_table_t{};
	context_table_t ctx;
	object out{root->eval(out_stream, &ctx)};
	return std::move(ctx);
}

object program::evaluate(context_table_t* ctx) const {
	if (!root) return object{};
	if (!ctx) ctx = &current_context->context_table();
	return object{root->eval(out_stream, ctx)};
}

context_table_t sym::execute(const std::string& code, std::ostream& out) {
	program prog = parse(code);
	prog.out_stream = &out;
	return std::move(prog.execute());
}

object sym::evaluate(const std::string& code, context_table_t* ctx, std::ostream& out) {
	program prog = parse(code);
	prog.out_stream = &out;
	object ret = prog.evaluate(ctx);
	out << ret.string() << std::endl;
	return std::move(ret);
}
