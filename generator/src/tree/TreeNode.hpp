#pragma once

#include <cstdint>
#include <vector>
#include "generator/Bitboard.hpp"

template <Bitboard T>
class TreeNode {
	public:
		TreeNode(T state) : state(state) {}
		T state;
		int32_t color = -1;
	
		void addChild(uint32_t child) {
			children.push_back(child);
		}

		const std::vector<uint32_t>& getChildren() const {
			return children;
		}

		void setColor(uint8_t c) {
			color = c;
		}

	private:
		std::vector<uint32_t> children;
};
