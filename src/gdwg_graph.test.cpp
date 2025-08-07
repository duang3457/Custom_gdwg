#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("2.2 Constructors - default") {
	auto g = gdwg::Graph<int, std::string>{};
	CHECK(g.empty());
}

TEST_CASE("2.2 Constructors - initializer list") {
	auto g = gdwg::Graph<int, std::string>{1, 2, 3};
	auto expected = std::vector<int>{1, 2, 3};
	CHECK(g.nodes() == expected);
}

TEST_CASE("2.2 Constructors - InputIt range") {
	auto vec = std::vector<int>{4, 5, 6};
	auto g = gdwg::Graph<int, std::string>(vec.begin(), vec.end());
	auto expected = std::vector<int>{4, 5, 6};
	CHECK(g.nodes() == expected);
}

TEST_CASE("2.2 Constructors - move constructor") {
	auto g1 = gdwg::Graph<int, std::string>{7, 8};
	auto g2 = gdwg::Graph<int, std::string>(std::move(g1));

	// g2 has content
	CHECK(g2.is_node(7));
	CHECK(g2.is_node(8));

	// g1 has been moved-from
	CHECK(g1.empty());
}

TEST_CASE("2.2 Constructors - move assignment") {
	auto g1 = gdwg::Graph<int, std::string>{9, 10};
	auto g2 = gdwg::Graph<int, std::string>{};
	g2 = std::move(g1);

	CHECK(g2.is_node(9));
	CHECK(g2.is_node(10));
	CHECK(g1.empty());
}

TEST_CASE("2.2 Constructors - copy constructor") {
	auto g1 = gdwg::Graph<int, std::string>{11, 12};
	auto g2 = gdwg::Graph<int, std::string>(g1);

	CHECK(g2.is_node(11));
	CHECK(g2.is_node(12));

	CHECK(g1.is_node(11));
	CHECK(g1.is_node(12));
}

TEST_CASE("2.2 Constructors - copy assignment") {
	auto g1 = gdwg::Graph<int, std::string>{13, 14};
	auto g2 = gdwg::Graph<int, std::string>{};
	g2 = g1;

	CHECK(g2.is_node(13));
	CHECK(g2.is_node(14));
	CHECK(g1.is_node(13));
	CHECK(g1.is_node(14));
}
