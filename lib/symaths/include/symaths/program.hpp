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

#include "symaths/context_table.hpp"
#include "symaths/detail/statement_node.hpp"

#include <iostream>
#include <vector>

namespace sym {
	class program;
	namespace detail {
		class statement_node;
	}

	class program {
	public:
		const detail::statement_node* root;
		std::ostream* out_stream;

		program(const detail::statement_node* node, std::ostream* p_stream = &std::cout);
		program(const std::vector<const detail::statement_node*>& statements, std::ostream* p_stream = &std::cout);

		// Executes the program in its own isolated context
		context_table_t execute() const;

		// Executes the program in the specified context (or global context if not specified)
		object evaluate(context_table_t* ctx = nullptr) const;

		[[nodiscard]] bool operator==(const program& other) const { return root == other.root; }
	};

	context_table_t execute(const std::string& code, std::ostream& out = std::cout);
	object evaluate(const std::string& code, context_table_t* ctx = nullptr, std::ostream& out = std::cout);
}

#endif
