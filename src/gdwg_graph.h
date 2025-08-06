#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H

#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace gdwg {
	// Forward declaration of Graph class
	template<typename N, typename E>
	class Graph;

	template<typename N, typename E>
	class Edge {
	 public:
		virtual ~Edge() = default;

		// 2.3.1
		virtual auto print_edge() const -> std::string = 0;
		virtual auto is_weighted() const noexcept -> bool = 0;
		virtual auto get_weight() const noexcept -> std::optional<E> = 0;
		virtual auto get_nodes() const noexcept -> std::pair<N, N> = 0;
		virtual auto operator==(Edge const& other) const noexcept -> bool = 0;

	 protected:
		friend class Graph<N, E>;

		Edge(N const& src, N const& dst)
		: src_{src}
		, dst_{dst} {}

		N src_;
		N dst_;
	};

	template<typename N, typename E>
	class WeightedEdge : public Edge<N, E> {
	 public:
		WeightedEdge(N const& src, N const& dst, E const& weight)
		: Edge<N, E>(src, dst)
		, weight_{weight} {}

		auto print_edge() const -> std::string override {
			// src -> dst | W | weight
			std::ostringstream oss;
			oss << this->src_ << " -> " << this->dst_ << " | W | " << weight_;
			return oss.str();
		}

		auto is_weighted() const noexcept -> bool override {
			return true;
		}

		auto get_weight() const noexcept -> std::optional<E> override {
			return weight_;
		}

		auto get_nodes() const noexcept -> std::pair<N, N> override {
			return {this->src_, this->dst_};
		}

		auto operator==(Edge<N, E> const& other) const noexcept -> bool override {
			if (!other.is_weighted())
				return false;
			auto [o_src, o_dst] = other.get_nodes();
			return (this->src_ == o_src) && (this->dst_ == o_dst) && (this->weight_ == other.get_weight().value());
		}

	 private:
		E weight_; // Weight of the edge
	};

	template<typename N, typename E>
	class UnweightedEdge : public Edge<N, E> {
	 public:
		UnweightedEdge(N const& src, N const& dst)
		: Edge<N, E>(src, dst) {}

		auto print_edge() const -> std::string override {
			// src -> dst | U
			std::ostringstream oss;
			oss << this->src_ << " -> " << this->dst_ << " | U";
			return oss.str();
		}

		auto is_weighted() const noexcept -> bool override {
			return false;
		}

		auto get_weight() const noexcept -> std::optional<E> override {
			return std::nullopt;
		}

		auto get_nodes() const noexcept -> std::pair<N, N> override {
			return {this->src_, this->dst_};
		}

		auto operator==(Edge<N, E> const& other) const noexcept -> bool override {
			if (other.is_weighted())
				return false;
			auto [o_src, o_dst] = other.get_nodes();
			return (this->src_ == o_src) && (this->dst_ == o_dst);
		}

	 private:
	};

	template<typename N, typename E>
	class Graph {
	 public:
		class iterator;
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

		// 2.4 Modifiers
		auto insert_node(N const& value) -> bool;
		auto insert_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool;
		auto replace_node(N const& old_data, N const& new_data) -> bool;
		auto merge_replace_node(N const& old_data, N const& new_data) -> void;
		auto erase_node(N const& value) -> bool;
		auto erase_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool;
		auto erase_edge(iterator i) -> iterator;
		auto erase_edge(iterator i, iterator s) -> iterator;
		auto clear() noexcept -> void;

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

	template<typename N, typename E>
	auto Graph<N, E>::is_node(N const& value) const -> bool {
		return nodes_.find(value) != nodes_.end();
	}

	template<typename N, typename E>
	auto Graph<N, E>::empty() const -> bool {
		return nodes_.empty();
	}

	template<typename N, typename E>
	auto Graph<N, E>::nodes() const -> std::vector<N> {
		return {nodes_.begin(), nodes_.end()};
	}

	template<typename N, typename E>
	auto Graph<N, E>::insert_node(N const& value) -> bool {
		return nodes_.insert(value).second;
	}

	template<typename N, typename E>
	auto Graph<N, E>::insert_edge(N const& src, N const& dst, std::optional<E> weight) -> bool {
		if (!is_node(src) || !is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::insert_edge when either src or dst node does not "
			                         "exist");
		}

		for (auto const& [from, edge_ptr] : edges_) {
			if (from == src) {
				auto [e_src, e_dst] = edge_ptr->get_nodes();
				if (e_dst == dst && edge_ptr->get_weight() == weight) {
					return false;
				}
			}
		}

		if (weight) {
			edges_.emplace(src, std::make_unique<WeightedEdge<N, E>>(src, dst, *weight));
		}
		else {
			edges_.emplace(src, std::make_unique<UnweightedEdge<N, E>>(src, dst));
		}
		return true;
	}

	template<typename N, typename E>
	auto Graph<N, E>::replace_node(N const& old_data, N const& new_data) -> bool {
		if (!is_node(old_data)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::replace_node on a node that doesn't exist");
		}
		if (is_node(new_data)) {
			return false;
		}

		nodes_.erase(old_data);
		nodes_.insert(new_data);

		for (auto& [from, edge_ptr] : edges_) {
			auto [src, dst] = edge_ptr->get_nodes();
			if (src == old_data)
				src = new_data;
			if (dst == old_data)
				dst = new_data;

			if (edge_ptr->is_weighted()) {
				edge_ptr = std::make_unique<WeightedEdge<N, E>>(src, dst, *edge_ptr->get_weight());
			}
			else {
				edge_ptr = std::make_unique<UnweightedEdge<N, E>>(src, dst);
			}
		}
		return true;
	}

} // namespace gdwg

#endif // GDWG_GRAPH_H
