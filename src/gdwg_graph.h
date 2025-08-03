#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H

#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace gdwg {
	template<typename N, typename E>
	class Graph;

	template<typename N, typename E>
	class Edge {
	 public:
		virtual ~Edge() = default;

	 private:
		friend class Graph<N, E>;
	};

	class WeightedEdge {
	 public:
	 private:
	};

	class UnweightedEdge {
	 public:
	 private:
	};

	template<typename N, typename E>
	class Graph {
	 public:
		// 2.2 Constructors
		Graph();
		Graph(std::initializer_list<N> il);
		template<typename InputIt>
		Graph(InputIt first, InputIt last);

		Graph(Graph&& other) noexcept;

		auto operator=(Graph&& other) noexcept -> Graph&;

		[[nodiscard]] auto is_node(N const& value) const -> bool;
		[[nodiscard]] auto empty() const -> bool;
		[[nodiscard]] auto nodes() const -> std::vector<N>;

		auto insert_node(N const& value) -> bool;

	 private:
		template<typename, typename>
		friend class Graph; // Allow Graph to access private members
	};

	template<typename N, typename E>
	Graph<N, E>::Graph() = default;

	template<typename N, typename E>
	Graph<N, E>::Graph(std::initializer_list<N> il)
	: Graph(il.begin(), il.end()) {}

	template<typename N, typename E>
	template<typename InputIt>
	Graph<N, E>::Graph(InputIt first, InputIt last) {
		for (auto it = first; it != last; ++it) {
			insert_node(*it);
		}
	}

} // namespace gdwg

#endif // GDWG_GRAPH_H
