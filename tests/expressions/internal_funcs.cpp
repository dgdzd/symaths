#include <gtest/gtest.h>

#include <symaths/symaths.hpp>
#include <symaths/utils/maths.hpp>

int main(int argc, char** argv) {
	sym::library lib{};
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(internal_funcs_test, find_node_paths) {
	sym::symbol x("x");
	sym::symbol y("y");
	sym::expression expr1 = x;
	sym::expression expr2 = 1 + x;
	sym::expression expr3 = 2 * x + 1;
	sym::expression expr4 = 2 - x;
	sym::expression expr5 = 3 * x + 6 * sym::pow(x, 2);
	sym::expression expr6 = 67 * y + 3;
	sym::expression expr7 = 3 * sym::pow(x, 2) + sym::cos(x) * sym::sin(x) * 2;

	auto p1 = sym::detail::search_node(expr1.root, x.ref);
	auto p2 = sym::detail::search_node(expr2.root, x.ref);
	auto p3 = sym::detail::search_node(expr3.root, x.ref);
	auto p4 = sym::detail::search_node(expr4.root, x.ref);
	auto p5 = sym::detail::search_node(expr5.root, x.ref);
	auto p6 = sym::detail::search_node(expr6.root, x.ref);
	auto p7 = sym::detail::search_node(expr7.root, x.ref);

	ASSERT_EQ(p1.size(), 1);
	ASSERT_EQ(p2.size(), 1);
	ASSERT_EQ(p3.size(), 1);
	ASSERT_EQ(p4.size(), 1);
	ASSERT_EQ(p5.size(), 2);
	ASSERT_EQ(p6.size(), 0);
	ASSERT_EQ(p7.size(), 3);
}

TEST(internal_funcs_test, find_symbols) {
	sym::symbol x("x");
	sym::symbol y("y");
	sym::symbol z("z");

	sym::expression expr1 = x;
	sym::expression expr2 = 2 * x + x;
	sym::expression expr3 = sym::pow(x, 2);
	sym::expression expr4 = x + y;
	sym::expression expr5 = (x + 3) * (y - 4);
	sym::expression expr6 = (x + 6) * (y - 7) + x;
	sym::expression expr7 = (x - 3) * (y + 2) + z + sym::pow(x, 4);

	auto s1 = sym::detail::list_symbols(expr1.root);
	auto s2 = sym::detail::list_symbols(expr2.root);
	auto s3 = sym::detail::list_symbols(expr3.root);
	auto s4 = sym::detail::list_symbols(expr4.root);
	auto s5 = sym::detail::list_symbols(expr5.root);
	auto s6 = sym::detail::list_symbols(expr6.root);
	auto s7 = sym::detail::list_symbols(expr7.root);

	ASSERT_EQ(s1.size(), 1);
	ASSERT_EQ(s2.size(), 1);
	ASSERT_EQ(s3.size(), 1);
	ASSERT_EQ(s4.size(), 2);
	ASSERT_EQ(s5.size(), 2);
	ASSERT_EQ(s6.size(), 2);
	ASSERT_EQ(s7.size(), 3);
}

TEST(internal_funcs_test, newton_binomial_coeffficients) {
	ASSERT_EQ(sym::utils::newton_binomial_coefficient(0, 1), 1);
	ASSERT_EQ(sym::utils::newton_binomial_coefficient(0, 2), 1);
	ASSERT_EQ(sym::utils::newton_binomial_coefficient(0, 38), 1);
	ASSERT_EQ(sym::utils::newton_binomial_coefficient(0, 813), 1);
	ASSERT_EQ(sym::utils::newton_binomial_coefficient(1, 32), 32);
	ASSERT_EQ(sym::utils::newton_binomial_coefficient(3, 13), 286);
}