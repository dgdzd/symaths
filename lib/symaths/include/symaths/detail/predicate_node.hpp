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

#ifndef SYM_NODE_PREDICATE_HPP
#define SYM_NODE_PREDICATE_HPP

#include <variant>
#include <vector>

namespace sym {
	namespace detail {
		class expression_node;

		struct equality {
			bool negated;
			std::vector<const expression_node*> expressions;

			bool operator==(const equality&) const = default;
		};

		struct inequality {
			enum type {
				LESS_EQUAL,
				LESS,
				GREATER_EQUAL,
				GREATER,
			};
			type type;
			bool negated;
			std::vector<const expression_node*> expressions;

			bool operator==(const inequality&) const = default;
		};

		struct congruence {
			bool negated;
			const expression_node* q;
			std::vector<const expression_node*> expressions;

			bool operator==(const congruence&) const = default;
		};

		struct element_inclusion {
			bool negated;
			const expression_node* element;
			const expression_node* set;

			bool operator==(const element_inclusion&) const = default;
		};

		struct set_inclusion {
			bool negated;
			const expression_node* subset;
			const expression_node* set;

			bool operator==(const set_inclusion&) const = default;
		};

		struct logical_or {
			bool negated;
			const expression_node* p;
			const expression_node* q;

			bool operator==(const logical_or&) const = default;
		};

		struct logical_and {
			bool negated;
			const expression_node* p;
			const expression_node* q;

			bool operator==(const logical_and&) const = default;
		};

		class predicate_node {
		public:
			using internal_data_t = std::variant<equality, inequality, congruence,
			element_inclusion, set_inclusion, logical_or, logical_and>;

			internal_data_t p_data;
			size_t p_hash = 0;
		};
	}
}

#endif
