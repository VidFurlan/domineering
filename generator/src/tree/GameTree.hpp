#pragma once

#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "tree/TreeNode.hpp"
#include "generator/Bitboard.hpp"

template <Bitboard T>
class GameTree {
	public:
		GameTree();
		~GameTree() = default;

		void addNode(const TreeNode<T>& node);
		TreeNode<T>& getNodeByIdx(uint32_t index);
		TreeNode<T>& getNodeByState(const T& state); 
		const int32_t findNode(const T& state) const;
		const size_t size() const { return nodes.size(); }

	private:
		std::vector<TreeNode<T>> nodes;
		std::unordered_map<T, uint32_t> nodeMap;
};

template <Bitboard T>
inline GameTree<T>::GameTree() {
	nodes.reserve(1ll<<20);
}

template <Bitboard T>
inline void GameTree<T>::addNode(const TreeNode<T>& node) {
	if (findNode(node.state) != -1) return;
	nodes.push_back(node);
	nodeMap[node.state] = nodes.size() - 1;
}

template <Bitboard T>
inline TreeNode<T>& GameTree<T>::getNodeByIdx(uint32_t index) {
	return nodes[index];
}

template <Bitboard T>
inline TreeNode<T>& GameTree<T>::getNodeByState(const T& state) {
	auto it = nodeMap.find(state);
	if (it != nodeMap.end()) {
		return nodes[it->second];
	}
	throw std::out_of_range("State not found in GameTree");
}

template <Bitboard T>
inline const int32_t GameTree<T>::findNode(const T& state) const {
	auto it = nodeMap.find(state);
	if (it != nodeMap.end()) {
		return it->second;
	}
	return -1;
}
