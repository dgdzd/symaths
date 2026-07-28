#include <gtest/gtest.h>

#include <symaths/symaths.hpp>
#include <symaths/polynomial.hpp>

int main(int argc, char** argv) {
	sym::library lib{};
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(parsing, lexer_tokenize) {
	sym::lexer lexer;
	ASSERT_NO_THROW(lexer.tokenize("val0 + val1 * 3val2( 3+ b)"));
}

TEST(parsing, parse_single) {
	sym::expression expr1 = sym::parse_single("3*x+2*y");
	sym::expression expr2 = sym::parse_single("3*(x+y+z)*4");
	sym::expression expr3 = sym::parse_single("3(x+y+z)*4");
	sym::expression expr4 = sym::parse_single("sin(x)");
	sym::expression expr5 = sym::parse_single("sin(x)cos(x)");
	sym::expression expr6 = sym::parse_single("sin(x)cos(x)+3a");
	sym::expression expr7 = sym::parse_single("sin(1+tan(x))x");
	sym::expression expr8 = sym::parse_single("cos(x)^2+sin(x)^2");

	ASSERT_EQ(expr1.string(), "3x+2y");
	ASSERT_EQ(expr2.string(), "3(x+y+z)*4");
	ASSERT_EQ(expr2, expr3);
	ASSERT_EQ(expr4.string(), "sin(x)");
	ASSERT_EQ(expr5.string(), "sin(x)cos(x)");
	ASSERT_EQ(expr6.string(), "sin(x)cos(x)+3a");
	ASSERT_EQ(expr7.string(), "sin(1+tan(x))x");
	ASSERT_EQ(expr8.string(), "cos(x)^2+sin(x)^2");
}

TEST(parsing, parse_multi) {
	const sym::context_table_t* context = sym::parse("3x+7;a(3b+7);(a+2)(a+3)");

	ASSERT_EQ(context->get_expressions().size(), 3);
}

TEST(parsing, named_objects) {
	sym::context_table_t* context = sym::parse("expr a: 3x+67");
	sym::expression a(0.0);

	ASSERT_NO_THROW(a = context->find_expression("a").value());

	ASSERT_EQ(context->get_expressions().size(), 1);
	ASSERT_EQ(a.string(), "3x+67");
}