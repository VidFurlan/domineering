#pragma once

#include <cstdint>
#include <vector>
#include "generator/Bitboard.hpp"

template <Bitboard T>
class TreeNode {
	public:
		TreeNode(T state) : state(state) {}
		T state;
	
		void addChild(uint32_t child) {
			children.push_back(child);
		}

		const std::vector<uint32_t>& getChildren() const {
			return children;
		}

	private:
		std::vector<uint32_t> children;
};
