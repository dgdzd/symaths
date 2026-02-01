#ifndef SYMATHS_LIBRARY_H
#define SYMATHS_LIBRARY_H

#include "base_operations.hpp"
#include "base_functions.hpp"

namespace sym {
	struct print_policies_t {
		struct sum_t {
			unsigned int operand_spaces = 0;
		};
		struct product_t {
			unsigned int operand_spaces = 0;
			bool use_stars_for_subexprs = false;
		};
		struct power_t {
			unsigned int operand_spaces_before = 0;
			unsigned int operand_spaces_after = 0;
		};

		sum_t sum;
		product_t product;
		power_t power;
	};

	extern print_policies_t print_policies;

	class expression;
	expression reduce(const expression&);
	expression sort(const expression&);
	expression develop(const expression&);

	class expression {
		detail::NodePtr root;

	public:
		expression(double val) : root(std::make_shared<objs::constant>(val)) {}
		expression(const std::string& name) : root(std::make_shared<objs::variable>(name)) {}
		expression(const char* name) : root(std::make_shared<objs::variable>(name)) {}
		expression(detail::NodePtr node) : root(std::move(node)) {}

		double operator()(const detail::Context& ctx) const { return root->eval(&ctx); }
		[[nodiscard]] std::string string() const { return root->string(nullptr); }

		/**
		 * @brief Checks if the expression is composed of constants only
		 *
		 * For example :
		 * - is_ground(3 + ln(4)) == true
		 * - is_ground(3 + 4x) == false
		 *
		 * @return Whether the expression is ground
		 */
		[[nodiscard]] bool is_ground() const { return root->is_ground(); }

		friend expression sym::reduce(const expression& expr);
		friend expression sym::sort(const expression& expr);
		friend expression sym::develop(const expression& expr);
		friend double get_power(const expression& expr);

		friend expression operator+(const expression& lhs, const expression& rhs) {
			auto add = std::make_shared<objs::addition>(lhs.root, rhs.root);
			add->flatten();
			return {add};
		}

		friend expression operator-(const expression& lhs, const expression& rhs) {
			auto negated_rhs = std::make_shared<objs::negate>(rhs.root);
			return lhs + expression(negated_rhs);
		}

		friend expression operator*(const expression& lhs, const expression& rhs) {
			auto mul = std::make_shared<objs::multiplication>(lhs.root, rhs.root);
			mul->flatten();
			return {mul};
		}

		friend expression operator/(const expression& lhs, const expression& rhs) {
			auto div = std::make_shared<objs::division>(lhs.root, rhs.root);
			div->flatten();
			return {div};
		}

		friend expression operator-(const expression& e) {
			return {std::make_shared<objs::negate>(e.root)};
		}

		friend expression pow(const expression& lhs, const expression& rhs);
	};

	inline expression pow(const expression& lhs, const expression& rhs) {
		return {std::make_shared<objs::power>(lhs.root, rhs.root)};
	}
}

#endif