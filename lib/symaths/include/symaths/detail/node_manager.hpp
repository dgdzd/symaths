#ifndef SYM_NODE_MANAGER_HPP
#define SYM_NODE_MANAGER_HPP

#include "expression_node.hpp"
#include "predicate_node.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sym {
	namespace detail {
		class expression_node;
		class predicate_node;
		class set_node;
	}

	template<typename T>
	struct node_key {
		typename T::internal_data_t data;

		bool operator==(const node_key& other) const {
			return data == other.data;
		}
	};

	struct expr_node_hash {
		std::size_t operator()(const node_key<detail::expression_node>& k) const;
	};

	struct pred_node_hash {
		std::size_t operator()(const node_key<detail::predicate_node>& k) const;
	};

	struct set_node_hash {
		std::size_t operator()(const node_key<detail::set_node>& k) const;
	};

	class node_manager_t {
		std::unordered_map<node_key<detail::expression_node>, const detail::expression_node*, expr_node_hash> expr_table;
		std::unordered_map<node_key<detail::predicate_node>,  const detail::predicate_node*,  pred_node_hash> pred_table;
		std::unordered_map<node_key<detail::set_node>,        const detail::set_node*,        set_node_hash>  set_table;

		std::vector<std::unique_ptr<detail::expression_node>> expr_arena;
		std::vector<std::unique_ptr<detail::predicate_node>>  pred_arena;
		std::vector<std::unique_ptr<detail::set_node>>        set_arena;

	public:
		const detail::expression_node* make_symbol(const std::string& name);
		const detail::expression_node* make_constant(const number& v);
		const detail::expression_node* make_constant(double v);
		const detail::expression_node* make_negation(const detail::expression_node* node);
		const detail::expression_node* make_add(const std::vector<const detail::expression_node*>& operands);
		const detail::expression_node* make_mul(const std::vector<const detail::expression_node*>& operands);
		const detail::expression_node* make_div(const detail::expression_node* a, const detail::expression_node* b);
		const detail::expression_node* make_pow(const detail::expression_node* b, const detail::expression_node* e);
		const detail::expression_node* make_func(uint32_t f_id, const std::vector<const detail::expression_node*>& args);
		const detail::expression_node* make_func(funcs::builtin_fn_id f_id, const std::vector<const detail::expression_node*>& args);

	private:
		const detail::expression_node* intern(detail::expression_node::internal_data_t data);
		const detail::predicate_node* intern(detail::predicate_node::internal_data_t data);
	};
}

#endif
