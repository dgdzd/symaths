#include <gtest/gtest.h>

#include <symaths/symaths.hpp>

int main(int argc, char** argv) {
	sym::library lib{};
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(basic_exprs_computing, simple_expressions) {
	sym::symbol x("x");
	sym::expression expr1 = 3 + x + 10;
	sym::expression expr2 = 3 * 4 * x;
	sym::expression expr3 = 3 + 4 * 3 * x;
	sym::expression expr4 = 3 + (2 + 7) * x;
	sym::expression expr5 = 2 * sym::pow(x, 2) + 3 * x + 6 * x;
	sym::expression expr6 = x - 3 - (x - 6);
	sym::expression expr7 = sym::pow(-3, -x);
	sym::expression expr8 = -3 * x * -6;
	sym::expression expr9 = -3 * x * -x;

	ASSERT_EQ(expr1.string(), "3+x+10");
	ASSERT_EQ(expr2.string(), "12x");
	ASSERT_EQ(expr3.string(), "3+12x");
	ASSERT_EQ(expr4.string(), "3+9x");
	ASSERT_EQ(expr5.string(), "2x^2+3x+6x");
	ASSERT_EQ(expr6.string(), "x-3-(x-6)");
	ASSERT_EQ(expr7.string(), "(-3)^(-x)");
	ASSERT_EQ(expr8.string(), "-3x*(-6)");
	ASSERT_EQ(expr9.string(), "-3x(-x)");
}

TEST(basic_exprs_computing, reduce_additions_simple) {
	sym::symbol x("x");
	sym::expression expr1 = 3 + x + 10;
	sym::expression expr2 = 3 * 4 * x;
	sym::expression expr3 = 3 + 4 * 3 * x;
	sym::expression expr4 = 3 + (2 + 7) * x;
	sym::expression expr5 = 2 * sym::pow(x, 2) + 3 * x + 6 * x;
	sym::expression expr6 = 2 * (3 + x) + 3 * (x + 3);

	ASSERT_EQ(sym::reduce(expr1).string(), "x+13");
	ASSERT_EQ(sym::reduce(expr2).string(), "12x");
	ASSERT_EQ(sym::reduce(expr3).string(), "12x+3");
	ASSERT_EQ(sym::reduce(expr4).string(), "9x+3");
	ASSERT_EQ(sym::reduce(expr5).string(), "2x^2+9x");
	ASSERT_EQ(sym::reduce(expr6).string(), "5(x+3)");
}

TEST(basic_exprs_computing, reduce_additions_multi) {
	sym::symbol a("a");
	sym::symbol x("x");
	sym::symbol y("y");
	sym::expression expr1 = a + a + x + x + y + y;
	sym::expression expr2 = 2 * a + a + 3 * y + 2 * x + a + 3 * x + x * y + y * x;

	ASSERT_EQ(sym::reduce(expr1).string(), "2a+2x+2y");
	ASSERT_EQ(sym::reduce(expr2).string(), "4a+5x+3y+2xy");
}

TEST(basic_exprs_computing, reduce_multiplications) {
	sym::symbol x("x");
	sym::symbol y("y");
	sym::expression expr1 = 3 * 6 * x;
	sym::expression expr2 = x * x;
	sym::expression expr3 = x * x * x;
	sym::expression expr4 = y * y * x * x;
	sym::expression expr5 = 5 * 5 * y * y * x * x;
	sym::expression expr6 = x * -x;
	sym::expression expr7 = x * -x * -x;

	ASSERT_EQ(sym::reduce(expr1).string(), "18x");
	ASSERT_EQ(sym::reduce(expr2).string(), "x^2");
	ASSERT_EQ(sym::reduce(expr3).string(), "x^3");
	ASSERT_EQ(sym::reduce(expr4).string(), "x^2y^2");
	ASSERT_EQ(sym::reduce(expr5).string(), "25x^2y^2");
	ASSERT_EQ(sym::reduce(expr6).string(), "-x^2");
	ASSERT_EQ(sym::reduce(expr7).string(), "x^3");
}

TEST(basic_exprs_computing, sort_expressions) {
	sym::symbol x("x");
	sym::symbol y("y");
	sym::expression z("z");
	sym::expression expr1 = 1 + x;
	sym::expression expr2 = y + x;
	sym::expression expr3 = z + y + x;
	sym::expression expr4 = 9 * x + 2 * sym::pow(x, 2);
	sym::expression expr5 = sym::pow(z, x * y + 3) + sym::pow(z, 3) + sym::pow(y, 2) + x;
	sym::expression expr6 = y + x + sym::pow(x, 2) + sym::pow(y, 2) + sym::pow(x * y, z);
	sym::expression expr7 = x * y + z * y + x + sym::pow(x, 2);

	ASSERT_EQ(sym::sort(expr1).string(), "x+1");
	ASSERT_EQ(sym::sort(expr2).string(), "x+y");
	ASSERT_EQ(sym::sort(expr3).string(), "x+y+z");
	ASSERT_EQ(sym::sort(expr4).string(), "2x^2+9x");
	ASSERT_EQ(sym::sort(expr5).string(), "z^(xy+3)+z^3+y^2+x");
	ASSERT_EQ(sym::sort(expr6).string(), "(xy)^z+x^2+y^2+x+y");
	ASSERT_EQ(sym::sort(expr7).string(), "x^2+x+xy+yz");
}

TEST(basic_exprs_computing, expand_products) {
	sym::symbol x("x");
	sym::expression expr1 = (x - 2) * (x + 2);
	sym::expression expr2 = (x - 2) * (x + 2) * (2 * x + 6);
	sym::expression expr3 = (x + x * (3 - x)) * (1 - x);

	ASSERT_EQ(sym::reduce(sym::expand(expr1)).string(), "x^2-4");
	ASSERT_EQ(sym::reduce(sym::expand(expr2)).string(), "2x^3+6x^2+-8x-24");
	ASSERT_EQ(sym::reduce(sym::expand(expr3)).string(), "x^3+-5x^2+4x");
}

TEST(basic_expr_computing, differentiate_base_operations) {
	sym::symbol x("x");
	sym::symbol y("y");
	sym::expression expr1 = 7;
	sym::expression expr2 = 2 * x;
	sym::expression expr3 = 7 + x + 3 * x + 6;
	sym::expression expr4 = x * y;
	sym::expression expr5 = 2 * x * y * 4;
	sym::expression expr6 = sym::pow(x, 2);
	sym::expression expr7 = sym::pow(x, 6);
	sym::expression expr8 = sym::pow(2, x);
	sym::expression expr9 = sym::pow(2, 3 * x);
	sym::expression expr10 = sym::pow(x, 3) + 2 * sym::pow(x, 2) - x;

	ASSERT_EQ(sym::differentiate(expr1, x).string(), "0");
	ASSERT_EQ(sym::differentiate(expr2, x).string(), "2");
	ASSERT_EQ(sym::differentiate(expr3, x).string(), "4");
	ASSERT_EQ(sym::differentiate(expr4, x).string(), "y");
	ASSERT_EQ(sym::differentiate(expr5, y).string(), "8x");
	ASSERT_EQ(sym::differentiate(expr6, x).string(), "2x");
	ASSERT_EQ(sym::differentiate(expr7, x).string(), "6x^5");
	ASSERT_EQ(sym::differentiate(expr8, x).string(), "0.6931471805599453*2^x");
	ASSERT_EQ(sym::differentiate(expr9, x).string(), "2.0794415416798357*2^(3x)");
	ASSERT_EQ(sym::differentiate(expr10, x).string(), "3x^2+4x-1");
}

TEST(basic_expr_computing, differentiate_builtin_functions) {
	sym::symbol x("x");
	sym::expression expr1 = sym::cos(x);
	sym::expression expr2 = sym::sin(2 * x);
	sym::expression expr3 = sym::tan(sym::pow(x, 2));
	sym::expression expr4 = sym::exp(x);
	sym::expression expr5 = sym::ln(3 * x);
	sym::expression expr6 = sym::sqrt(5 * sym::pow(x, 2));

	ASSERT_EQ(sym::differentiate(expr1, x).string(), "-sin(x)");
	ASSERT_EQ(sym::differentiate(expr2, x).string(), "2cos(2x)");
	ASSERT_EQ(sym::differentiate(expr3, x).string(), "2x(tan(x^2)^2+1)");
	ASSERT_EQ(sym::differentiate(expr4, x), expr4);
	ASSERT_EQ(sym::differentiate(expr5, x).string(), "3(3x)^(-1)");
	ASSERT_EQ(sym::differentiate(expr6, x).string(), "10x(2sqrt(5x^2))^(-1)");
}



TEST(basic_exprs_computing, lexer_tokenize) {
	sym::lexer lexer;
	lexer.tokenize("val0 + val1 * 3val2( 3+ b)");
}