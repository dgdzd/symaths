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

#ifndef SYM_NODE_STATEMENT_HPP
#define SYM_NODE_STATEMENT_HPP

#include "symaths/detail/types.hpp"

#include <optional>
#include <optional>
#include <optional>
#include <string>
#include <variant>
#include <variant>
#include <variant>
#include <vector>


namespace sym {
	class context_table_t;

	namespace detail {
		class mathexpr_node;
		class predicate_node;
		class set_node;
		class statement_node;

		struct stmt_sequence {
			std::vector<const statement_node*> statements;

			void eval(context_table_t* ctx) const;

			bool operator==(const stmt_sequence&) const = default;
		};

		struct assignment {
			std::string varname;
			const statement_node* value;
			value_type_t expected_type;

			expression_value_t eval(context_table_t* ctx) const;

			bool operator==(const assignment&) const = default;
		};

		struct expression_statement {
			expression_value_t expr;

			expression_value_t eval(context_table_t* ctx) const;

			bool operator==(const expression_statement&) const = default;
		};

		struct conditional {
			const statement_node* condition; // MUST return an expression_value_t
			const statement_node* then_branch;
			const statement_node* else_branch;

			bool operator==(const conditional&) const = default;
		};

		/*struct while_loop {
			const predicate_node* condition;
			const statement_node* body;

			bool operator==(const while_loop&) const = default;
		};

		struct for_loop {
			const expression_node* variable;
			const set_node* iterable;
			const statement_node* body;

			bool operator==(const for_loop&) const = default;
		};*/

		struct function_definition {
			std::string name;
			std::vector<std::string> params;
			const statement_node* body;

			bool operator==(const function_definition&) const = default;
		};

		class statement_node {
		public:
			using internal_data_t = std::variant<stmt_sequence, assignment, expression_statement,
				conditional, /*while_loop, for_loop,*/ function_definition>;

			internal_data_t p_data;
			size_t p_hash = 0;

			[[nodiscard]] std::optional<expression_value_t> eval(std::ostream* p_out, context_table_t* ctx = nullptr) const;
			[[nodiscard]] value_type_t return_type() const;

			statement_node() = default;
			virtual ~statement_node() = default;
		};
	}
}


#endif
