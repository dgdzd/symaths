#ifndef SYM_EXPRESSIONS_HPP
#define SYM_EXPRESSIONS_HPP

#include "detail/nodes.h"

#include <format>
#include <string>
#include <type_traits>
#include <vector>

namespace sym::detail {
	// Base class for all N-ary operation nodes
	class n_operation : public node {
	protected:
		std::vector<NodePtr> operands;

	public:
		n_operation() = delete;
		n_operation(std::shared_ptr<n_operation>&& rhv) : node(rhv->kind(), rhv->priority()), operands(std::move(rhv->operands)) {}
		n_operation(const std::shared_ptr<n_operation>& rhv) : node(rhv->kind(), rhv->priority()), operands(std::move(rhv->operands)) {}

		template <typename... Args>
		explicit n_operation(kind_ kind, unsigned int priority, Args&&... args) : node(kind, priority), operands{std::forward<Args>(args)...} {}
		n_operation(kind_ kind, unsigned int priority, const std::vector<NodePtr>& operands) : node(kind, priority), operands(operands) {}

		void add_operand(NodePtr operand);
		[[nodiscard]] const std::vector<NodePtr>& get_operands() const;
		NodePtr operator[](unsigned int index) const;

		[[nodiscard]] virtual ptr<n_operation> sorted() const = 0;
		virtual void flatten() = 0;
	};
}

namespace sym::objs {
	// Leaf Node: variables
	class variable : public detail::node {
		std::string m_name;

	public:
		explicit variable(std::string n) : node(kind_::variable, ~0), m_name(std::move(n)) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;
		[[nodiscard]] const std::string& name() const;
	};

	// Leaf Node: constants
	class constant : public detail::node {
		double value;

	public:
		constant(double v) : node(kind_::constant, ~0), value(v) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;
	};

	// N+ary Node: addition
	class addition : public detail::n_operation {
	public:
		explicit addition(std::vector<detail::NodePtr> ops) : n_operation(kind_::addition, 1, std::move(ops)) {}

		template<typename... Args>
		explicit addition(Args&&... args) : n_operation(kind_::addition, 1, std::forward<Args>(args)...) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;
		[[nodiscard]]  ptr<n_operation> sorted() const override;
		void flatten() override;

		[[nodiscard]] detail::NodePtr reduced();
	};

	class negate : public detail::node {
		detail::NodePtr child;
	public:
		explicit negate(detail::NodePtr c) : node(kind_::negate, 0), child(std::move(c)) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;

		[[nodiscard]] detail::NodePtr get_child() const;
	};

	// N+ary Node: multiplication
	class multiplication : public detail::n_operation {
	public:
		explicit multiplication(std::vector<detail::NodePtr> ops) : n_operation(kind_::multiplication, 2, std::move(ops)) {}

		template<typename... Args>
		explicit multiplication(Args&&... args) : n_operation(kind_::multiplication, 2, std::forward<Args>(args)...) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;
		[[nodiscard]]  ptr<n_operation> sorted() const override;
		void flatten() override;

		[[nodiscard]] detail::AdditionNodePtr unroll() const;
	};

	// N+ary Node: division
	// TODO : maybe delete this class, and replace with power(expression, constant(-1))
	class division : public detail::n_operation {
		std::vector<detail::NodePtr> operands;
	public:
		explicit division(std::vector<detail::NodePtr> ops) : n_operation(kind_::division, 2, std::move(ops)) {}

		template<typename... Args>
		explicit division(Args&&... args) : n_operation(kind_::division, 2, std::forward<Args>(args)...) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;
		[[nodiscard]] ptr<n_operation> sorted() const override;
		void flatten() override;
	};

	class power : public detail::node {
		detail::NodePtr m_base;
		detail::NodePtr m_exp;

	public:
		power() = delete;
		power(detail::NodePtr base, detail::NodePtr exponent) : node(kind_::power, 3), m_base(std::move(base)), m_exp(std::move(exponent)) {}

		[[nodiscard]] double eval(const detail::Context* ctx) const override;
		[[nodiscard]] std::string string(const node* parent) const override;
		[[nodiscard]] bool is_ground() const override;
		[[nodiscard]] const detail::NodePtr& base() const;
		[[nodiscard]] const detail::NodePtr& exponent() const;
	};
}
#endif
