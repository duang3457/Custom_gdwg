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
		std::set<N> nodes_; // Set of nodes in the graph
		std::multimap<N, std::unique_ptr<gdwg::Edge<N, E>>> edges_; // Map of edges, where each edge is associated with
		                                                            // a node

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

	template<typename N, typename E>
	auto Graph<N, E>::insert_node(N const& value) -> bool {
		return nodes_.insert(value).second;
	}

	template<typename N, typename E>
	auto Graph<N, E>::is_node(N const& value) const -> bool {
		return nodes_.find(value) != nodes_.end();
	}

	template<typename N, typename E>
	Graph<N, E>::Graph(Graph&& other) noexcept
	: nodes_{std::move(other.nodes_)}
	, edges_{std::move(other.edges_)} {
		// Leave other in a valid but empty state
		other.nodes_.clear();
		other.edges_.clear();
	}

	template<typename N, typename E>
	auto Graph<N, E>::operator=(Graph&& other) noexcept -> Graph<N, E>& {
		if (this != &other) {
			nodes_ = std::move(other.nodes_);
			edges_ = std::move(other.edges_);
			other.nodes_.clear();
			other.edges_.clear();
		}
		return *this;
	}

} // namespace gdwg

#endif // GDWG_GRAPH_H
