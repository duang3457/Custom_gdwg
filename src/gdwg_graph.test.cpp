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

TEST_CASE("2.5 Accessors - is_node, empty, nodes") {
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

TEST_CASE("2.5 Accessors - edges() returns correct edge list") {
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

TEST_CASE("2.5 Accessors - connections returns sorted destination nodes") {
	auto g = gdwg::Graph<std::string, int>{"a", "b", "c", "d"};
	g.insert_edge("a", "b");
	g.insert_edge("a", "c", 3);
	g.insert_edge("a", "d", 2);

	auto conns = g.connections("a");
	CHECK(conns == std::vector<std::string>{"b", "c", "d"});

	CHECK_THROWS_WITH(g.connections("x"), "Cannot call gdwg::Graph<N, E>::connections if src doesn't exist in the graph");
}

TEST_CASE("2.5 Accessors - find returns correct iterator") {
	auto g = gdwg::Graph<std::string, int>{"p", "q"};
	g.insert_edge("p", "q", 7);
	g.insert_edge("p", "q");

	auto it = g.find("p", "q", 7);
	REQUIRE(it != g.end());
	auto [from1, to1, weight1] = *it;
	CHECK(from1 == "p");
	CHECK(to1 == "q");
	CHECK(weight1 == 7);

	auto it2 = g.find("p", "q");
	REQUIRE(it2 != g.end());
	auto [from2, to2, weight2] = *it2;
	CHECK(weight2 == std::nullopt);

	auto it3 = g.find("q", "p", 1);
	CHECK(it3 == g.end()); // not found
}

using graph = gdwg::Graph<int, int>;
using triple = std::tuple<int, int, std::optional<int>>;

static auto collect_all(graph const& g) -> std::vector<triple> {
	std::vector<triple> out;
	for (auto it = g.begin(); it != g.end(); ++it) {
		auto v = *it; // v.from, v.to, v.weight
		out.emplace_back(v.from, v.to, v.weight);
	}
	return out;
}

TEST_CASE("2.6 Iterator Access: begin()/end() basic") {
	SECTION("empty graph: begin() == end()") {
		graph g;
		REQUIRE(g.begin() == g.end());
	}

	SECTION("nodes but no edges: begin() == end()") {
		graph g;
		g.insert_node(1);
		g.insert_node(2);
		REQUIRE(g.begin() == g.end());
	}

	SECTION("single edge: begin points to the first edge, ++ reaches end") {
		graph g;
		g.insert_node(1);
		g.insert_node(2);
		REQUIRE(g.insert_edge(1, 2)); // unweighted

		auto it = g.begin();
		REQUIRE(it != g.end());

		auto v = *it;
		CHECK(v.from == 1);
		CHECK(v.to == 2);
		CHECK_FALSE(v.weight.has_value());

		++it;
		REQUIRE(it == g.end());
	}
}

TEST_CASE("2.6 Iterator Access: iteration order and const correctness") {
	graph g;
	for (int x : {1, 2, 3})
		g.insert_node(x);

	REQUIRE(g.insert_edge(1, 1)); // U
	REQUIRE(g.insert_edge(1, 1, 2)); // W 2
	REQUIRE(g.insert_edge(1, 2, 4)); // W 4
	REQUIRE(g.insert_edge(1, 2, 3)); // W 3
	// src=2
	REQUIRE(g.insert_edge(2, 1)); // U
	REQUIRE(g.insert_edge(2, 1, -5)); // W -5
	REQUIRE(g.insert_edge(2, 2, 0)); // W 0
	// src=3
	REQUIRE(g.insert_edge(3, 3)); // U

	// Expected traversal: src asc, then dst asc, U before W, then weights asc
	std::vector<triple> expected{
	    {1, 1, std::nullopt},
	    {1, 1, 2},
	    {1, 2, 3},
	    {1, 2, 4},
	    {2, 1, std::nullopt},
	    {2, 1, -5},
	    {2, 2, 0},
	    {3, 3, std::nullopt},
	};

	SECTION("non-const begin()/end() produce expected sequence") {
		auto got = collect_all(g);
		REQUIRE(got == expected);
	}

	SECTION("const begin()/end() also work and produce same sequence") {
		graph const& cg = g;
		auto got = collect_all(cg);
		REQUIRE(got == expected);
	}
}

using G = gdwg::Graph<std::string, int>;
using T = std::tuple<std::string, std::string, std::optional<int>>;

static auto make(std::vector<std::string> nodes, std::vector<T> edges) -> G {
	G g;
	for (auto const& n : nodes)
		g.insert_node(n);
	for (auto const& [s, d, w] : edges) {
		if (w)
			g.insert_edge(s, d, *w);
		else
			g.insert_edge(s, d);
	}
	return g;
}

TEST_CASE("2.7 operator== (simple)") {
	SECTION("empty graphs equal") {
		G a, b;
		CHECK(a == b);
	}

	SECTION("same nodes+edges equal regardless of insertion order") {
		auto a = make({"a", "b", "c"}, {{"a", "b", std::nullopt}, {"a", "b", 2}, {"c", "c", std::nullopt}});
		auto b = make({"c", "b", "a"}, {{"c", "c", std::nullopt}, {"a", "b", 2}, {"a", "b", std::nullopt}});
		CHECK(a == b);
	}

	SECTION("different node set -> not equal") {
		auto a = make({"a", "b"}, {});
		auto b = make({"a", "b", "x"}, {});
		CHECK_FALSE(a == b);
	}

	SECTION("edge differences -> not equal") {
		auto base = make({"a", "b", "c"}, {{"a", "b", std::nullopt}, {"a", "b", 3}, {"c", "c", std::nullopt}});
		// missing one edge
		auto miss = make({"a", "b", "c"}, {{"a", "b", std::nullopt}, {"c", "c", std::nullopt}});
		CHECK_FALSE(base == miss);
		// different weight
		auto diffw = make({"a", "b", "c"}, {{"a", "b", std::nullopt}, {"a", "b", 4}, {"c", "c", std::nullopt}});
		CHECK_FALSE(base == diffw);
		// unweighted vs weighted
		auto uw_vs_w = make({"a", "b", "c"}, {{"a", "b", 0}, {"a", "b", 3}, {"c", "c", std::nullopt}});
		CHECK_FALSE(base == uw_vs_w);
	}
}

TEST_CASE("2.8 Extractor: empty graph") {
	gdwg::Graph<std::string, int> g;
	std::ostringstream oss;
	oss << g;
	auto out = oss.str();
	CHECK((out.empty() || out == "()" || out == "()\n"));
}

TEST_CASE("2.8 Extractor: formatted output and ordering") {
	gdwg::Graph<std::string, int> g;

	// nodes
	g.insert_node("A");
	g.insert_node("B");
	g.insert_node("C");
	g.insert_node("Z");

	g.insert_edge("A", "B", 2); // W 2
	g.insert_edge("A", "B"); // U
	g.insert_edge("A", "A", -1); // W -1
	g.insert_edge("A", "A"); // U
	g.insert_edge("B", "C", 5); // W 5

	std::ostringstream oss;
	oss << g;

	auto const expected = std::string{
	    R"(A (
  A -> A | U
  A -> A | W | -1
  A -> B | U
  A -> B | W | 2
)
B (
  B -> C | W | 5
)
C (
)
Z (
)
)"};

	CHECK(oss.str() == expected);
}

using G2 = gdwg::Graph<int, int>;
using Triple2 = std::tuple<int, int, std::optional<int>>;

static std::vector<Triple2> collect(G2 const& g) {
	std::vector<Triple2> v;
	for (auto it = g.begin(); it != g.end(); ++it) {
		auto x = *it;
		v.emplace_back(x.from, x.to, x.weight);
	}
	return v;
}

TEST_CASE("2.9 iterator (simple)") {
	G2 g;
	for (int n : {1, 2, 3, 99})
		g.insert_node(n);
	g.insert_edge(1, 1); // U
	g.insert_edge(1, 1, 2); // W 2
	g.insert_edge(2, 1); // U

	std::vector<Triple2> expected{
	    {1, 1, std::nullopt},
	    {1, 1, 2},
	    {2, 1, std::nullopt},
	};

	CHECK(collect(g) == expected);

	auto it = g.end();
	--it;
	auto [f, t, w] = *it;
	CHECK(f == 2);
	CHECK(t == 1);
	CHECK_FALSE(w.has_value());

	G2 const& cg = g;
	CHECK(collect(cg) == expected);
}

TEST_CASE("2.10: graph owns resources (nodes/edges outlive caller)") {
	gdwg::Graph<std::string, int> g;

	{
		std::string a = "Hello";
		std::string b = "World";
		int w = 3;

		g.insert_node(a);
		g.insert_node(b);
		g.insert_edge(a, b, w);

		// Modifying the original variable should not affect the graph
		a = "HELLO";
		w = 999;
	} // a, b, w go out of scope and are destroyed

	CHECK(g.is_node("Hello"));
	CHECK(g.is_node("World"));

	auto es = g.edges("Hello", "World");
	REQUIRE(es.size() == 1);
	REQUIRE(es[0]->get_weight().has_value());
	CHECK(*es[0]->get_weight() == 3);

	// Inserted temporary objects should also be owned
	CHECK(g.insert_node(std::string{"Temp"}));
	CHECK(g.is_node("Temp"));
}

TEST_CASE("2.10: nodes()/edges() return copies (mutating copies doesn't affect graph)") {
	gdwg::Graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "B");

	// nodes() is a copy
	auto ns = g.nodes();
	ns.push_back("ZZZ"); // Change return value
	CHECK(g.nodes() == std::vector<std::string>{"A", "B"});

	// edges() is a copy (edge objects are also copies)
	auto es = g.edges("A", "B");
	auto original_size = es.size();
	es.clear(); // Change return value
	CHECK(g.edges("A", "B").size() == original_size);
}
