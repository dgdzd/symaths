#include <gtest/gtest.h>

#include <symaths/symaths.hpp>

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(basic_exprs_computing, simple_addition) {
	sym::expression x("x");
	sym::expression expr = 3 + sym::pow(x, 2+x) + 10;

	ASSERT_EQ(expr.string(), "3 + x^(2 + x) + 10");
}

/*TEST(ct_expressions_value_test, simple_addition) {
	using namespace sym::objs;
	auto expr = sum(
		value(1),
		value(2)
	);

	ASSERT_EQ(expr.eval(), 3);
}

TEST(ct_expressions_value_test, simple_multiplication) {
	using namespace sym::ct::objs;
	auto expr = multiplication(
		value(6),
		value(8)
	);

	ASSERT_EQ(expr.eval(), 48);
}

TEST(ct_expressions_value_test, simple_division) {
	using namespace sym::ct::objs;
	auto expr = division(
		value(10),
		value(5)
	);

	ASSERT_EQ(expr.eval(), 2);
}

TEST(ct_expressions_value_test, simple_power) {
	using namespace sym::ct::objs;
	auto expr = power(
		value(2),
		value(8)
	);

	ASSERT_EQ(expr.eval(), 256);
}

// 3^3 + 3*3
TEST(ct_expressions_value_test, medium_calc1) {
	using namespace sym::ct::objs;
	auto expr = sum(
		power(
			value(3),
			value(3)
		),
		multiplication(
			value(3),
			value(3)
		)
	);

	ASSERT_EQ(expr.eval(), 36);
}

TEST(ct_expressions_value_test, long_calc1) {
	using namespace sym::ct::objs;

	auto x = value(3.0);

	auto expr = sum(
		value(3.0),
		multiplication(
			value(2.0),
			x,
			sum(
				power(x, value(2.0)),
				multiplication(value(5.0), x),
				value(-3.0)
			)
		),
		ln(
			multiplication(value(3.0), x)
		)
	);

	std::cout << "With x = 3.0 --> 3 + 2x(x^2 + 5x - 3) + ln(3x) = " << expr.eval() << std::endl;
	auto real_val = 3 + 2*x.eval() * (std::pow(x.eval(), 2) + 5*x.eval() - 3) + std::log(3*x.eval());
	ASSERT_EQ(real_val, expr.eval());
}*/