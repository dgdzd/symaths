#ifndef SYMATHS_LIBRARY_H
#define SYMATHS_LIBRARY_H

#include "base_operations.hpp"
#include "base_functions.hpp"

namespace sym {
	class expression {
		detail::NodePtr root;

	public:
		expression(double val) : root(std::make_shared<objs::constant>(val)) {}
		expression(const std::string& name) : root(std::make_shared<objs::variable>(name)) {}
		expression(const char* name) : root(std::make_shared<objs::variable>(name)) {}
		expression(detail::NodePtr node) : root(std::move(node)) {}

		double operator()(const detail::Context& ctx) const { return root->eval(&ctx); }
		[[nodiscard]] std::string string() const { return root->string(nullptr); }
		[[nodiscard]] bool is_ground() const { return root->is_ground(); }

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

		/*friend expression operator*(const expression& lhs, double rhs) {
			auto d = std::make_shared<objs::constant>(rhs);
			return expression(std::make_shared<objs::multiplication>(lhs.root, d));
		}

		friend expression operator*(double lhs, const expression& rhs) {
			auto d = std::make_shared<objs::constant>(lhs);
			return expression(std::make_shared<objs::multiplication>(d, rhs.root));
		}*/

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