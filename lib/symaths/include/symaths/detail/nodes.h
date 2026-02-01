#ifndef SYM_NODE_H
#define SYM_NODE_H

#include "fwd_decls.hpp"

#include <unordered_map>
#include <memory>
#include <string>

namespace sym {
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
			unsigned int m_priority;

		public:
			node() = delete;
			explicit node(unsigned int priority) : m_priority(priority) {}
		    virtual ~node() = default;

			[[nodiscard]] unsigned int priority() const { return m_priority; }

		    [[nodiscard]] virtual double eval(const Context* ctx) const = 0;
			[[nodiscard]] virtual std::string string(const node* parent) const = 0;
			[[nodiscard]] virtual bool is_ground() const = 0;
		};

	}
}

#endif
