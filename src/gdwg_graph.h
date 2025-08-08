#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H

#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
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
	std::unique_ptr<Edge<N, E>> clone_edge(Edge<N, E> const& edge) {
		auto [src, dst] = edge.get_nodes();
		if (edge.is_weighted()) {
			return std::make_unique<WeightedEdge<N, E>>(src, dst, edge.get_weight().value());
		}
		else {
			return std::make_unique<UnweightedEdge<N, E>>(src, dst);
		}
	}

	template<typename N, typename E>
	class Graph {
	 public:
		class iterator;

		auto begin() const -> iterator;
		auto end() const -> iterator;

		// 2.2 Constructors
		Graph();
		Graph(std::initializer_list<N> il);
		template<typename InputIt>
		Graph(InputIt first, InputIt last);

		Graph(Graph&& other) noexcept;
		auto operator=(Graph&& other) noexcept -> Graph&;

		Graph(Graph const& other);
		auto operator=(Graph const& other) -> Graph&;

		// 2.5 Accessors
		[[nodiscard]] auto is_node(N const& value) const -> bool;
		[[nodiscard]] auto empty() const -> bool;
		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool;
		[[nodiscard]] auto nodes() const -> std::vector<N>;
		[[nodiscard]] auto edges(N const& src, N const& dst) const -> std::vector<std::unique_ptr<Edge<N, E>>>;
		[[nodiscard]] auto find(N const& src, N const& dst, std::optional<E> weight = std::nullopt) const -> iterator;
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N>;

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

		// 2,7 Comparisons
		[[nodiscard]] auto operator==(Graph const& other) const -> bool;

		// 2.8 Extractor 
		template<typename NN, typename EE>
		friend auto operator<<(std::ostream& os, Graph<NN, EE> const& g) -> std::ostream&;

	 private:
		struct EdgeKey {
			N src;
			N dst;
			bool is_weighted;
			std::optional<E> weight;
			bool operator==(EdgeKey const&) const = default;
		};

		struct EdgeKeyLess {
			bool operator()(EdgeKey const& a, EdgeKey const& b) const {
				if (a.src != b.src)
					return a.src < b.src;
				if (a.dst != b.dst)
					return a.dst < b.dst;
				if (a.is_weighted != b.is_weighted)
					return a.is_weighted < b.is_weighted;
				return a.weight < b.weight;
			}
		};

		std::set<N> nodes_; // Set of nodes in the graph
		std::map<EdgeKey, std::unique_ptr<gdwg::Edge<N, E>>, EdgeKeyLess> edges_;

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
	gdwg::Graph<N, E>::Graph(Graph const& other) {
		nodes_ = other.nodes_;

		for (auto const& [src, edge_ptr] : other.edges_) {
			edges_.insert({src, clone_edge(*edge_ptr)});
		}
	}

	template<typename N, typename E>
	auto gdwg::Graph<N, E>::operator=(Graph const& other) -> Graph<N, E>& {
		if (this != &other) {
			nodes_ = other.nodes_;

			edges_.clear();

			for (auto const& [src, edge_ptr] : other.edges_) {
				edges_.insert({src, clone_edge(*edge_ptr)});
			}
		}
		return *this;
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

		EdgeKey key{src, dst, weight.has_value(), weight};
		if (edges_.find(key) != edges_.end())
			return false;

		if (weight) {
			edges_.emplace(key, std::make_unique<WeightedEdge<N, E>>(src, dst, *weight));
		}
		else {
			edges_.emplace(key, std::make_unique<UnweightedEdge<N, E>>(src, dst));
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

	template<typename N, typename E>
	auto Graph<N, E>::merge_replace_node(N const& old_data, N const& new_data) -> void {
		if (!is_node(old_data) || !is_node(new_data)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::merge_replace_node on old or new data if they "
			                         "don't exist in the graph");
		}

		std::vector<std::tuple<N, N, std::optional<E>>> to_add;

		for (auto const& [from, edge_ptr] : edges_) {
			auto [src, dst] = edge_ptr->get_nodes();
			if (src == old_data)
				src = new_data;
			if (dst == old_data)
				dst = new_data;
			to_add.emplace_back(src, dst, edge_ptr->get_weight());
		}

		erase_node(old_data);

		for (auto const& [src, dst, w] : to_add) {
			insert_edge(src, dst, w);
		}
	}

	template<typename N, typename E>
	auto Graph<N, E>::erase_node(N const& value) -> bool {
		if (!is_node(value)) {
			return false;
		}
		nodes_.erase(value);

		for (auto it = edges_.begin(); it != edges_.end();) {
			auto [src, dst] = it->second->get_nodes();
			if (src == value || dst == value) {
				it = edges_.erase(it);
			}
			else {
				++it;
			}
		}
		return true;
	}

	// by key
	template<typename N, typename E>
	auto Graph<N, E>::erase_edge(N const& src, N const& dst, std::optional<E> weight) -> bool {
		if (!is_node(src) || !is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::erase_edge on src or dst if they don't exist in "
			                         "the graph");
		}
		EdgeKey key{src, dst, weight.has_value(), weight};
		auto it = edges_.find(key);
		if (it == edges_.end())
			return false;
		edges_.erase(it);
		return true;
	}

	// by iterator
	template<typename N, typename E>
	auto Graph<N, E>::erase_edge(iterator i) -> iterator {
		auto it = i.current_;
		if (it == edges_.cend())
			return this->end();
		auto next = std::next(it);
		edges_.erase(it);
		return iterator{next};
	}

	// by range
	template<typename N, typename E>
	auto Graph<N, E>::erase_edge(iterator first, iterator last) -> iterator {
		edges_.erase(first.current_, last.current_);
		return last;
	}

	template<typename N, typename E>
	auto Graph<N, E>::clear() noexcept -> void {
		nodes_.clear();
		edges_.clear();
	}

	template<typename N, typename E>
	auto Graph<N, E>::is_node(N const& value) const -> bool {
		return nodes_.contains(value);
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
	auto Graph<N, E>::is_connected(N const& src, N const& dst) const -> bool {
		if (!is_node(src) || !is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::is_connected if src or dst node don't exist in "
			                         "the graph");
		}
		typename Graph<N, E>::EdgeKey lb{src, dst, false, std::nullopt};
		for (auto it = edges_.lower_bound(lb); it != edges_.end() && it->first.src == src && it->first.dst == dst; ++it)
		{
			return true;
		}
		return false;
	}

	template<typename N, typename E>
	auto Graph<N, E>::edges(N const& src, N const& dst) const -> std::vector<std::unique_ptr<Edge<N, E>>> {
		if (!is_node(src) || !is_node(dst)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::edges if src or dst node don't exist in the "
			                         "graph");
		}
		std::vector<std::unique_ptr<Edge<N, E>>> out;
		EdgeKey lb{src, dst, false, std::nullopt};
		for (auto it = edges_.lower_bound(lb); it != edges_.end() && it->first.src == src && it->first.dst == dst; ++it)
		{
			auto const& ep = it->second;

			if (ep->is_weighted()) {
				out.push_back(std::make_unique<WeightedEdge<N, E>>(src, dst, *ep->get_weight()));
			}
			else {
				out.push_back(std::make_unique<UnweightedEdge<N, E>>(src, dst));
			}
		}
		return out;
	}

	template<typename N, typename E>
	auto Graph<N, E>::find(N const& src, N const& dst, std::optional<E> weight) const -> iterator {
		EdgeKey key{src, dst, weight.has_value(), weight};
		auto it = edges_.find(key);
		return (it == edges_.end()) ? this->end() : iterator{it};
	}

	template<typename N, typename E>
	auto Graph<N, E>::connections(N const& src) const -> std::vector<N> {
		if (!is_node(src)) {
			throw std::runtime_error("Cannot call gdwg::Graph<N, E>::connections if src doesn't exist in the graph");
		}
		std::set<N> dsts;

		typename Graph<N, E>::EdgeKey lb{src, N{}, false, std::nullopt};
		for (auto it = edges_.lower_bound(lb); it != edges_.end() && it->first.src == src; ++it) {
			dsts.insert(it->first.dst);
		}
		return {dsts.begin(), dsts.end()};
	}

	template<typename N, typename E>
	auto Graph<N, E>::begin() const -> iterator {
		return iterator{edges_.cbegin()};
	}

	template<typename N, typename E>
	auto Graph<N, E>::end() const -> iterator {
		return iterator{edges_.cend()};
	}

	// 2,7 Comparisons
	template<typename N, typename E>
	auto Graph<N, E>::operator==(Graph const& other) const -> bool {
		if (nodes_ != other.nodes_) {
			return false;
		}

		if (edges_.size() != other.edges_.size()) {
			return false;
		}

		auto it1 = edges_.cbegin();
		auto it2 = other.edges_.cbegin();
		for (; it1 != edges_.cend(); ++it1, ++it2) {
			if (it1->first != it2->first) {
				return false;
			}
			if (!(*(it1->second) == *(it2->second))) {
				return false;
			}
		}

		return true;
	}

	// 2.8 Extractor
	template<typename N, typename E>
	auto operator<<(std::ostream& os, Graph<N, E> const& g) -> std::ostream& {
		for (auto const& src : g.nodes_) {
			os << src << " (\n";
			N current_dst{};
			bool has_current = false;
			auto flush_edge_line = [&](Edge<N, E> const& ep) {
				auto [f, t] = ep.get_nodes();
				os << "  " << f << " -> " << t;
				if (ep.is_weighted()) {
					os << " | W | " << *ep.get_weight() << "\n";
				}
				else {
					os << " | U\n";
				}
			};

			typename Graph<N, E>::EdgeKey lb{src, N{}, false, std::nullopt};
			for (auto it = g.edges_.lower_bound(lb); it != g.edges_.end() && it->first.src == src; ++it) {
				if (!has_current || it->first.dst != current_dst) {
					current_dst = it->first.dst;
					has_current = true;
				}
				flush_edge_line(*it->second);
			}
			os << ")\n";
		}
		return os;
	}

	// 2.9 Iterator class
	template<typename N, typename E>
	class Graph<N, E>::iterator {
	 public:
		using value_type = struct {
			N from;
			N to;
			std::optional<E> weight;
		};
		using reference = value_type;
		using pointer = void;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;

		iterator() = default;

		using UnderIt = typename std::map<EdgeKey, std::unique_ptr<Edge<N, E>>, EdgeKeyLess>::const_iterator;
		explicit iterator(UnderIt it)
		: current_{it} {}

		auto operator*() const -> reference {
			auto const& edge_ptr = current_->second;
			auto [e_src, e_dst] = edge_ptr->get_nodes();
			return {e_src, e_dst, edge_ptr->get_weight()};
		}

		auto operator++() -> iterator& {
			++current_;
			return *this;
		}

		auto operator++(int) -> iterator {
			auto temp = *this;
			++*this;
			return temp;
		}

		auto operator--() -> iterator& {
			--current_;
			return *this;
		}

		auto operator--(int) -> iterator {
			auto temp = *this;
			--*this;
			return temp;
		}

		auto operator==(iterator const& other) const -> bool {
			return current_ == other.current_;
		}

	 private:
		friend class Graph<N, E>;
		UnderIt current_;
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H
