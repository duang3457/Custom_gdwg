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

TEST_CASE("2.3 Edge hierarchy - print_edge and weight") {
	auto we = gdwg::WeightedEdge<std::string, int>("A", "B", 10);
	auto ue = gdwg::UnweightedEdge<std::string, int>("A", "B");

	CHECK(we.print_edge() == "A -> B | W | 10");
	CHECK(ue.print_edge() == "A -> B | U");
	CHECK(we.is_weighted() == true);
	CHECK(ue.is_weighted() == false);
	CHECK(we.get_weight() == 10);
	CHECK(ue.get_weight() == std::nullopt);
	CHECK(we.get_nodes() == std::pair<std::string, std::string>{"A", "B"});
	CHECK(ue.get_nodes() == std::pair<std::string, std::string>{"A", "B"});
}

TEST_CASE("2.4 Modifiers - insert_node and insert_edge") {
	auto g = gdwg::Graph<std::string, int>{};
	CHECK(g.insert_node("A"));
	CHECK(g.insert_node("B"));
	CHECK_FALSE(g.insert_node("A")); // already exists

	CHECK(g.insert_edge("A", "B", 15)); // weighted
	CHECK(g.insert_edge("A", "B")); // unweighted
	CHECK_FALSE(g.insert_edge("A", "B", 15)); // duplicate weighted
	CHECK_FALSE(g.insert_edge("A", "B")); // duplicate unweighted

	CHECK_THROWS_WITH(g.insert_edge("A", "C", 12),
	                  "Cannot call gdwg::Graph<N, E>::insert_edge when either src or dst node does not exist");
}

TEST_CASE("2.5 is_node, empty, nodes") {
	auto g = gdwg::Graph<std::string, int>{};
	CHECK(g.empty());

	g.insert_node("a");
	g.insert_node("b");
	g.insert_node("c");

	CHECK(g.is_node("a"));
	CHECK_FALSE(g.is_node("x"));
	CHECK_FALSE(g.empty());

	CHECK(g.nodes() == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("2.5 is_connected") {
	auto g = gdwg::Graph<std::string, int>{"x", "y", "z"};
	g.insert_edge("x", "y");
	g.insert_edge("x", "z", 42);

	CHECK(g.is_connected("x", "y")); // unweighted
	CHECK(g.is_connected("x", "z")); // weighted
	CHECK_FALSE(g.is_connected("y", "x"));

	CHECK_THROWS_WITH(g.is_connected("a", "b"),
	                  "Cannot call gdwg::Graph<N, E>::is_connected if src or dst node don't exist in the graph");
}

TEST_CASE("2.5 edges() returns correct edge list") {
	auto g = gdwg::Graph<std::string, int>{"x", "y"};
	g.insert_edge("x", "y");
	g.insert_edge("x", "y", 1);
	g.insert_edge("x", "y", 5);

	auto result = g.edges("x", "y");

	// Should have 3 edges: 1 unweighted, then weighted 1 and 5 (sorted)
	REQUIRE(result.size() == 3);
	CHECK(result[0]->is_weighted() == false);
	CHECK(result[1]->get_weight() == 1);
	CHECK(result[2]->get_weight() == 5);

	CHECK_THROWS_WITH(g.edges("x", "z"),
	                  "Cannot call gdwg::Graph<N, E>::edges if src or dst node don't exist in the graph");
}