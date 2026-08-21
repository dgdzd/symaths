/*
 *	                            _   _
 *	  ___ _   _ _ __ ___   __ _| |_| |__  ___
 *	 / __| | | | '_ ` _ \ / _` | __| '_ \/ __|   Symbolic maths for C++
 *	 \__ \ |_| | | | | | | (_| | |_| | | \__ \   Version : 0.0.1
 *	 |___/\__, |_| |_| |_|\__,_|\__|_| |_|___/   https://github.com/dgdzd/symaths
 *		  |___/
 *
 * All source code is distributed under the GNU General Public License v2.0.
 *
 */

#ifndef SYM_NODE_MANAGER_HPP
#define SYM_NODE_MANAGER_HPP

#include "symaths/base_functions.hpp"
#include "symaths/detail/mathexpr_node.hpp"
#include "symaths/detail/predicate_node.hpp"
#include "symaths/detail/set_node.hpp"
#include "symaths/detail/statement_node.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sym {
	namespace detail {
		class mathexpr_node;
		class predicate_node;
		class set_node;
		class statement_node;
	}

	template<typename T>
	struct node_key {
		typename T::internal_data_t data;

		bool operator==(const node_key& other) const {
			return data == other.data;
		}
	};

	struct expr_node_hash {
		std::size_t operator()(const node_key<detail::mathexpr_node>& k) const;
	};

	struct pred_node_hash {
		std::size_t operator()(const node_key<detail::predicate_node>& k) const;
	};

	struct set_node_hash {
		std::size_t operator()(const node_key<detail::set_node>& k) const;
	};

	struct statement_node_hash {
		std::size_t operator()(const node_key<detail::statement_node>& k) const;
	};

	class node_manager_t {
		std::unordered_map<node_key<detail::mathexpr_node>, const detail::mathexpr_node*, expr_node_hash> expr_table;
		std::unordered_map<node_key<detail::predicate_node>,  const detail::predicate_node*,  pred_node_hash> pred_table;
		std::unordered_map<node_key<detail::set_node>,        const detail::set_node*,        set_node_hash>  set_table;
		std::unordered_map<node_key<detail::statement_node>,  const detail::statement_node*,  statement_node_hash> stmt_table;

		std::vector<std::unique_ptr<detail::mathexpr_node>>   expr_arena;
		std::vector<std::unique_ptr<detail::predicate_node>>  pred_arena;
		std::vector<std::unique_ptr<detail::set_node>>        set_arena;
		std::vector<std::unique_ptr<detail::statement_node>>  stmt_arena;

	public:
		// Make expression
		const detail::mathexpr_node* make_symbol(const std::string& name);
		const detail::mathexpr_node* make_constant(const number& v);
		const detail::mathexpr_node* make_constant(double v);
		const detail::mathexpr_node* make_negation(const detail::mathexpr_node* node);
		const detail::mathexpr_node* make_add(const std::vector<const detail::mathexpr_node*>& operands);
		const detail::mathexpr_node* make_mul(const std::vector<const detail::mathexpr_node*>& operands);
		const detail::mathexpr_node* make_div(const detail::mathexpr_node* a, const detail::mathexpr_node* b);
		const detail::mathexpr_node* make_pow(const detail::mathexpr_node* b, const detail::mathexpr_node* e);
		const detail::mathexpr_node* make_func(uint32_t f_id, const std::vector<const detail::mathexpr_node*>& args);
		const detail::mathexpr_node* make_func(funcs::builtin_fn_id f_id, const std::vector<const detail::mathexpr_node*>& args);
		const detail::mathexpr_node* make_builtin_call(uint32_t id, const std::vector<detail::expression_value_t>& args);

		// Make predicate
		const detail::predicate_node* make_equal(const std::vector<const detail::mathexpr_node*>& members, bool negated = false);
		const detail::predicate_node* make_inequal(detail::inequality::kind type, const std::vector<const detail::mathexpr_node*>& members, bool negated = false);
		const detail::predicate_node* make_congruence(const detail::mathexpr_node* mod, const std::vector<const detail::mathexpr_node*>& members, bool negated = false);
		const detail::predicate_node* make_element_inclusion(const detail::set_node* set, const detail::mathexpr_node* elem, bool negated = false);
		const detail::predicate_node* make_set_inclusion(const detail::set_node* set, const detail::set_node* subset, bool negated = false);
		const detail::predicate_node* make_or(const detail::predicate_node* p, const detail::predicate_node* q, bool negated = false);
		const detail::predicate_node* make_and(const detail::predicate_node* p, const detail::predicate_node* q, bool negated = false);

		// Make statement
		const detail::statement_node* make_sequence(const std::vector<const detail::statement_node*>& statements);
		const detail::statement_node* make_assignment(const std::string& name, const detail::statement_node* value, detail::value_type_t expected_type);
		const detail::statement_node* make_assignment(const std::string& name, const detail::mathexpr_node* value);
		const detail::statement_node* make_assignment(const std::string& name, const detail::predicate_node* value);
		const detail::statement_node* make_assignment(const std::string& name, const detail::set_node* value);
		const detail::statement_node* make_expression_statement(const detail::expression_value_t& expr);
		const detail::statement_node* make_expression_statement(const detail::mathexpr_node* expr);
		const detail::statement_node* make_expression_statement(const detail::predicate_node* expr);
		const detail::statement_node* make_expression_statement(const detail::set_node* expr);
		const detail::statement_node* make_conditional(const detail::statement_node* condition, const detail::statement_node* then_branch, const detail::statement_node* else_branch);
		/*const detail::statement_node* make_while_loop(const detail::predicate_node* condition, const detail::statement_node* body);
		const detail::statement_node* make_for_loop(const detail::mathexpr_node* variable, const detail::set_node* iterable, const detail::statement_node* body);*/
		const detail::statement_node* make_function_definition(const std::string& name, const std::vector<std::string>& params, const detail::statement_node* body);

	private:
		const detail::mathexpr_node* intern(detail::mathexpr_node::internal_data_t data);
		const detail::predicate_node* intern(detail::predicate_node::internal_data_t data);
		const detail::set_node* intern(detail::set_node::internal_data_t data);
		const detail::statement_node* intern(detail::statement_node::internal_data_t data);
	};
}

#endif
