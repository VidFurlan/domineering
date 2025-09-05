#pragma once

#include <cstdint>
#include <iostream>
#include <stack>
#include <tuple>
#include <unordered_map>
#include "generator/AGenerator.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class GeneratorMinimax : public AGenerator<T, W, H> {
	public:
		GeneratorMinimax() = default;
		~GeneratorMinimax() = default;

		void run();
};

template <Bitboard T, std::uint8_t W, std::uint8_t H>
inline void GeneratorMinimax<T, W, H>::run() {
	std::unordered_map<T, bool> color;

	{
		std::stack<std::tuple<T, int32_t, uint32_t>> stk;
		stk.push({this->m_initial, -1, 0});
		while (!stk.empty()) {
			auto [current, parent, turn] = stk.top();
			stk.pop();


			if (this->m_tree.findNode(current) != -1) {
				uint32_t idx = this->m_tree.findNode(current);
				this->m_tree.getNodeByIdx(parent).addChild(idx);
				continue;
			}

			this->m_tree.addNode(TreeNode<T>(current));
			uint32_t idx = this->m_tree.size() - 1;

			if (parent != -1) this->m_tree.getNodeByIdx(parent).addChild(idx);

			uint32_t possible = 0;
			uint32_t empty = 0;
			for (uint32_t i = 0; i < this->c_size; i++) {
				empty += ((current >> i) & T{1}) == T{0};
			}

			uint32_t full = this->c_size - empty;
			for (uint8_t i = 0; i < H; ++i) {
				for (uint8_t j = 0; j < W; ++j) {
					if (!GET_BIT(current, W, i, j)) {
						if (turn % 2 == 0 && i + 1 < H && !GET_BIT(current, W, i + 1, j)) {
							T next = current;
							SET_BIT(next, W, i, j);
							SET_BIT(next, W, i + 1, j);
							stk.push({next, idx, turn ^ 1});
							possible++;
						}
						if (turn % 2 == 1 && j + 1 < W && !GET_BIT(current, W, i, j + 1)) {
							T next = current;
							SET_BIT(next, W, i, j);
							SET_BIT(next, W, i, j + 1);
							stk.push({next, idx, turn ^ 1});
							possible++;
						}
					}
				}
			}


			if (possible == 0) {
				color[current] = turn ^ 1;
			}
		}
	}

	{
		auto dfs = [&](auto dfs, uint32_t current, uint32_t turn) -> void {
			if (color.find(current) != color.end()) return;

			bool win = false;
			for (auto child_idx : this->m_tree.getNodeByState(current).getChildren()) {
				T child = this->m_tree.getNodeByIdx(child_idx).state;
				dfs(dfs, child, turn ^ 1);
				if (color.find(child) != color.end() && color[child] == turn) {
					win = true;
					break;
				}
			}

			color[current] = win ? turn : (turn ^ 1);
		};

		dfs(dfs, this->m_initial, 0);
	}

	std::cout << "Wins: " << (color[this->m_initial] == 0 ? "First" : "Second") << " player\n";
}
