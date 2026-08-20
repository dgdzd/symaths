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

#ifndef SYM_NODE_EXPRESSION_HPP
#define SYM_NODE_EXPRESSION_HPP

#include "symaths/numbers.hpp"
#include "symaths/detail/types.hpp"

#include <deque>
#include <unordered_map>
#include <string>
#include <variant>

namespace sym {
	class node_manager_t;

	namespace detail {
		class mathexpr_node;
	}

	namespace detail {
		using Context = std::unordered_map<std::string, number>;

		struct symbol {
			static constexpr unsigned int priority = ~0;
			std::string name;

			bool operator==(const symbol&) const = default;
		};

		struct constant {
			static constexpr unsigned int priority = ~0;
			number value;

			bool operator==(const constant&) const = default;
		};

		struct expr_negation {
			static constexpr unsigned int priority = 2;
			const mathexpr_node* child;

			[[nodiscard]] const mathexpr_node* sorted() const;
			[[nodiscard]] const mathexpr_node* reduced() const;
			[[nodiscard]] const mathexpr_node* expanded() const;

			bool operator==(const expr_negation&) const = default;
		};

		struct addition {
			static constexpr unsigned int priority = 1;
			std::vector<const mathexpr_node*> operands;

			[[nodiscard]] const mathexpr_node* sorted() const;
			[[nodiscard]] const mathexpr_node* reduced() const;
			[[nodiscard]] const mathexpr_node* expanded() const;

			bool operator==(const addition&) const = default;
		};

		struct multiplication {
			static constexpr unsigned int priority = 2;
			std::vector<const mathexpr_node*> operands;

			[[nodiscard]] const mathexpr_node* sorted() const;
			[[nodiscard]] const mathexpr_node* reduced() const;
			[[nodiscard]] const mathexpr_node* expanded() const;

			bool operator==(const multiplication&) const = default;
		};

		struct power {
			static constexpr unsigned int priority = 3;
			const mathexpr_node* base;
			const mathexpr_node* exponent;

			[[nodiscard]] const mathexpr_node* reduced() const;
			[[nodiscard]] const mathexpr_node* expanded() const;

			bool operator==(const power&) const = default;
		};

		struct function_call {
			static constexpr unsigned int priority = 0;
			uint32_t f_id;
			std::vector<const mathexpr_node*> args;

			bool operator==(const function_call&) const = default;
		};

		struct builtin_call {
			static constexpr unsigned int priority = 0;
			uint32_t id;
			std::vector<expression_value_t> args;

			bool operator==(const builtin_call&) const = default;
		};


		// Base node
		class mathexpr_node {
		public:
			using internal_data_t = std::variant<symbol, constant, expr_negation, addition, multiplication, power, function_call, builtin_call>;

			internal_data_t p_data;
			size_t p_hash = 0;

			mathexpr_node() = default;
			virtual ~mathexpr_node() = default;

			[[nodiscard]] unsigned int priority() const;
			[[nodiscard]] number eval(const Context* ctx) const;
			[[nodiscard]] const mathexpr_node* resolve_builtins() const;
			[[nodiscard]] std::string string(const mathexpr_node* parent = nullptr, bool first = true) const;
			[[nodiscard]] bool is_ground() const;
			[[nodiscard]] bool depends_on(const mathexpr_node* n) const;
		};

		using node_path_t = std::deque<const mathexpr_node*>;

		std::vector<node_path_t> search_node(const mathexpr_node* parent, const mathexpr_node* to_search);
		std::vector<const mathexpr_node*> list_symbols(const mathexpr_node* parent);
	}
}

#endif
