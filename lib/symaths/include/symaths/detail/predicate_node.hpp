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
		class set_node;
		class expression_node;
		class predicate_node;

		struct pred_negation {
			const predicate_node* p;

			bool operator==(const pred_negation&) const = default;
		};

		struct equality {
			std::vector<const expression_node*> expressions;

			bool operator==(const equality&) const = default;
		};

		struct inequality {
			enum kind {
				LESS_EQUAL,
				LESS,
				GREATER_EQUAL,
				GREATER,
			};
			kind type;
			std::vector<const expression_node*> expressions;

			bool operator==(const inequality&) const = default;
		};

		struct congruence {
			const expression_node* mod;
			std::vector<const expression_node*> expressions;

			bool operator==(const congruence&) const = default;
		};

		struct element_inclusion {
			const expression_node* element;
			const set_node* set;

			bool operator==(const element_inclusion&) const = default;
		};

		struct set_inclusion {
			const set_node* subset;
			const set_node* set;

			bool operator==(const set_inclusion&) const = default;
		};

		struct logical_or {
			const predicate_node* p;
			const predicate_node* q;

			bool operator==(const logical_or&) const = default;
		};

		struct logical_and {
			const predicate_node* p;
			const predicate_node* q;

			bool operator==(const logical_and&) const = default;
		};


		class predicate_node {
		public:
			using internal_data_t = std::variant<pred_negation, equality, inequality, congruence,
			element_inclusion, set_inclusion, logical_or, logical_and>;

			internal_data_t p_data;
			size_t p_hash = 0;

			predicate_node() = default;
			virtual ~predicate_node() = default;

			[[nodiscard]] bool is_ground() const;
		};
	}
}

#endif
