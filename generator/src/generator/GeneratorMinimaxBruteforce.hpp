#pragma once

#include <cstdint>
#include <iostream>
#include <stack>
#include <tuple>
#include "generator/AGenerator.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class GeneratorMinimaxBruteforce : public AGenerator<T, W, H> {
	public:
		GeneratorMinimaxBruteforce() = default;
		~GeneratorMinimaxBruteforce() = default;

		void run();

	private:
		void dfsGetAnswer(uint32_t idx, uint32_t turn);
};

template <Bitboard T, std::uint8_t W, std::uint8_t H>
inline void GeneratorMinimaxBruteforce<T, W, H>::dfsGetAnswer(uint32_t idx, uint32_t turn) {
		if (this->m_tree.getNodeByIdx(idx).color != -1) return;

		bool win = false;
		for (auto child_idx : this->m_tree.getNodeByIdx(idx).getChildren()) {
			dfsGetAnswer(child_idx, turn ^ 1);
			if (this->m_tree.getNodeByIdx(child_idx).color == turn) {
				win = true;
				break;
			}
		}

		this->m_tree.getNodeByIdx(idx).setColor(win ? turn : (turn ^ 1));
}

template <Bitboard T, std::uint8_t W, std::uint8_t H>
inline void GeneratorMinimaxBruteforce<T, W, H>::run() {
	std::stack<std::tuple<T, int32_t, uint32_t>> stk;
	stk.push({this->m_initial, -1, 0});

	int count = 0;
	while (!stk.empty()) {
		auto [current, parent, turn] = stk.top();
		stk.pop();

		count++;

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
			this->m_tree.getNodeByIdx(idx).setColor(turn ^ 1);
		}
	}

	dfsGetAnswer(0, 0);

	std::cout << "States:" << count << std::endl;
	std::cout << "Wins: " << (this->m_tree.getNodeByIdx(0).color == 0 ? "First" : "Second") << " player\n";
}
