#ifndef SYM_NODE_H
#define SYM_NODE_H

#include <unordered_map>
#include <memory>
#include <string>
#include <variant>

/*namespace sym {
	namespace detail {
		class node;
	}

	namespace detail {
		using Context = std::unordered_map<std::string, double>;

		using NodePtr = ptr<node>;
		using VariableNodePtr = ptr<objs::variable>;
		using ConstantNodePtr = ptr<objs::constant>;
		using AdditionNodePtr = ptr<objs::addition>;
		using MultiplicationNodePtr = ptr<objs::multiplication>;
		using DivisionNodePtr = ptr<objs::division>;
		using PowerNodePtr = ptr<objs::power>;

		// Base node
		class node {
		public:
			enum class kind : int {
				variable, addition, multiplication, division, power, function, constant
			};

		private:
			kind m_kind;
			unsigned int m_priority;
			bool m_negated = false;

		public:
			node() = delete;
			explicit node(kind kind, unsigned int priority) : m_kind(kind), m_priority(priority) {}
		    virtual ~node() = default;

			[[nodiscard]] unsigned int priority() const { return m_priority; }
			[[nodiscard]] kind type() const { return m_kind; }
			[[nodiscard]] bool& negated() { return m_negated; }
			[[nodiscard]] bool negated() const { return m_negated; }

		    [[nodiscard]] virtual double eval(const Context* ctx) const = 0;
			[[nodiscard]] virtual std::string string(const node* parent, bool first = true) const = 0;
			[[nodiscard]] virtual bool is_ground() const = 0;
		};

	}
}*/

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


		// Base node
		class node {
		public:
			using internal_data_t = std::variant<symbol, constant, negation, addition, multiplication, power>;
			friend node_manager_t;

			internal_data_t p_data;
			size_t p_hash = 0;

			node() = default;
			virtual ~node() = default;

			[[nodiscard]] unsigned int priority() const;
			[[nodiscard]] double eval(const Context* ctx) const;
			[[nodiscard]] std::string string(const node* parent, bool first = true) const;
			[[nodiscard]] bool is_ground() const;
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

	private:
		const detail::node* intern(detail::node::internal_data_t data);
	};
}

#endif
