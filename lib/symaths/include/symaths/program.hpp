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

#ifndef SYM_PROGRAM_HPP
#define SYM_PROGRAM_HPP

#include "context_table.hpp"
#include "symaths/detail/statement_node.hpp"

#include <vector>

namespace sym {
	template <typename T>
	concept is_expression_object = requires(T t)
	{
		requires std::is_pointer_v<decltype(t.root)>;
	};

	class program;
	namespace detail {
		class statement_node;
	}

	// Friendly wrapper around std::optional<detail::expression_value_t>
	class program_output {
		std::optional<detail::expression_value_t> m_root;

		program_output() = default;
		explicit program_output(const std::optional<detail::expression_value_t>& root) : m_root(root) {}

		friend program;

	public:
		template <is_expression_object T>
		T cast() const {
			if (m_root.has_value()) {
				return T(std::get<decltype(T::root)>(m_root.value()));
			}
			throw std::logic_error{"Cannot cast null program_output."};
		}

		template <typename T>
		T cast() const {
			if (m_root.has_value()) {
				return std::get<T>(m_root.value());
			}
			throw std::logic_error{"Cannot cast null program_output."};
		}

		std::string string();
	};

	class program {
	public:
		const detail::statement_node* root;

		program(const detail::statement_node* node);
		program(const std::vector<const detail::statement_node*>& statements);

		// Executes the program in its own isolated context
		program_output execute() const;

		// Executes the program in the specified context (or global context if not specified)
		program_output evaluate(context_table_t* ctx = nullptr) const;

		[[nodiscard]] bool operator==(const program& other) const { return root == other.root; }
	};
}

#endif
