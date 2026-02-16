#ifndef SYM_NODE_HPP
#define SYM_NODE_HPP

#include "symaths/base_functions.hpp"

#include <unordered_map>
#include <memory>
#include <string>
#include <variant>

namespace sym {
	class node_manager_t;

	namespace detail {
		class node;
	}

	namespace detail {
		using Context = std::unordered_map<std::string, double>;

		struct symbol {
			static constexpr unsigned int priority = ~0;
			std::string name;

			bool operator==(const symbol&) const = default;
		};

		struct constant {
			static constexpr unsigned int priority = ~0;
			double value;

			bool operator==(const constant&) const = default;
		};

		struct negation {
			static constexpr unsigned int priority = 2;
			const node* child;

			[[nodiscard]] const node* sorted() const;
			[[nodiscard]] const node* reduced() const;
			[[nodiscard]] const node* expanded() const;

			bool operator==(const negation&) const = default;
		};

		struct addition {
			static constexpr unsigned int priority = 1;
			std::vector<const node*> operands;

			[[nodiscard]] const node* sorted() const;
			[[nodiscard]] const node* reduced() const;
			[[nodiscard]] const node* expanded() const;

			bool operator==(const addition&) const = default;
		};

		struct multiplication {
			static constexpr unsigned int priority = 2;
			std::vector<const node*> operands;

			[[nodiscard]] const node* sorted() const;
			[[nodiscard]] const node* reduced() const;
			[[nodiscard]] const node* expanded() const;

			bool operator==(const multiplication&) const = default;
		};

		struct power {
			static constexpr unsigned int priority = 3;
			const node* base;
			const node* exponent;

			[[nodiscard]] const node* reduced() const;
			[[nodiscard]] const node* expanded() const;

			bool operator==(const power&) const = default;
		};

		struct function_call {
			static constexpr unsigned int priority = 0;
			uint32_t f_id;
			std::vector<const node*> args;

			bool operator==(const function_call&) const = default;
		};


		// Base node
		class node {
		public:
			using internal_data_t = std::variant<symbol, constant, negation, addition, multiplication, power, function_call>;
			friend node_manager_t;

			internal_data_t p_data;
			size_t p_hash = 0;

			node() = default;
			virtual ~node() = default;

			[[nodiscard]] unsigned int priority() const;
			[[nodiscard]] double eval(const Context* ctx) const;
			[[nodiscard]] std::string string(const node* parent, bool first = true) const;
			[[nodiscard]] bool is_ground() const;
			[[nodiscard]] bool depends_on(const node* n) const;
		};
	}

	struct node_key {
		detail::node::internal_data_t data;

		bool operator==(const node_key& other) const {
			return data == other.data;
		}
	};

	struct node_hash {
		std::size_t operator()(const node_key& k) const;
	};

	class node_manager_t {
		std::unordered_map<node_key, const detail::node*, node_hash> table;
		std::vector<std::unique_ptr<detail::node>> arena;

	public:
		const detail::node* make_symbol(const std::string& name);
		const detail::node* make_constant(double v);
		const detail::node* make_negation(const detail::node* node);
		const detail::node* make_add(const std::vector<const detail::node*>& operands);
		const detail::node* make_mul(const std::vector<const detail::node*>& operands);
		const detail::node* make_div(const detail::node* a, const detail::node* b);
		const detail::node* make_pow(const detail::node* b, const detail::node* e);
		const detail::node* make_func(uint32_t f_id, const std::vector<const detail::node*>& args);
		const detail::node* make_func(funcs::builtin_fn_id f_id, const std::vector<const detail::node*>& args);

	private:
		const detail::node* intern(detail::node::internal_data_t data);
	};
}

#endif
