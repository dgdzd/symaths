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
	sym::expression expr1 = sym::parse_expression("3*x+2*y");
	sym::expression expr2 = sym::parse_expression("3*(x+y+z)*4");
	sym::expression expr3 = sym::parse_expression("3(x+y+z)*4");
	sym::expression expr4 = sym::parse_expression("sin(x)");
	sym::expression expr5 = sym::parse_expression("sin(x)cos(x)");
	sym::expression expr6 = sym::parse_expression("sin(x)cos(x)+3a");
	sym::expression expr7 = sym::parse_expression("sin(1+tan(x))x");
	sym::expression expr8 = sym::parse_expression("cos(x)^2+sin(x)^2");
	sym::expression expr9 = sym::parse_expression("2x^2");

	ASSERT_EQ(expr1.string(), "3x+2y");
	ASSERT_EQ(expr2.string(), "3(x+y+z)*4");
	ASSERT_EQ(expr2, expr3);
	ASSERT_EQ(expr4.string(), "sin(x)");
	ASSERT_EQ(expr5.string(), "sin(x)cos(x)");
	ASSERT_EQ(expr6.string(), "sin(x)cos(x)+3a");
	ASSERT_EQ(expr7.string(), "sin(1+tan(x))x");
	ASSERT_EQ(expr8.string(), "cos(x)^2+sin(x)^2");
	ASSERT_EQ(expr9.string(), "2x^2");
}

TEST(parsing, parse_multi) {
	sym::context_table_t ctx;
	sym::program prog = sym::parse("3x+7;a(3b+7);(a+2)(a+3)");

	ASSERT_NO_THROW(prog.evaluate(&ctx));
}

TEST(parsing, named_objects) {
	sym::program prog = sym::parse("expr a: 3x+67");
	sym::expression a;

	ASSERT_NO_THROW(a = prog.evaluate());

	ASSERT_EQ(a.string(), "3x+67");
}

TEST(parsing, program_exec) {
	sym::symbol x("x");

	sym::program prog1 = sym::parse("expr a: reduce(3x+7x)+reduce(3+7)");
	sym::program prog2 = sym::parse("differentiate(x^3+2x^2-x, x)");
	sym::expression expr2 = sym::pow(x, 3) + 2 * sym::pow(x, 2) - x;

	sym::expression a1;
	sym::expression a2;

	ASSERT_NO_THROW(a1 = prog1.evaluate());
	ASSERT_EQ(a1.string(), "10x+10");
	ASSERT_NO_THROW(a2 = prog2.evaluate());
	ASSERT_EQ(sym::differentiate(expr2, x).string(), "3x^2+4x-1");
	ASSERT_EQ(sym::differentiate(expr2, x), a2);
}