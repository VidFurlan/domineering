#pragma once

#include <cstdint>
#include <iostream>
#include <stack>
#include <tuple>
#include "generator/AGenerator.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class GeneratorMinimaxMoveOrdering : public AGenerator<T, W, H> {
	public:
		GeneratorMinimaxMoveOrdering() = default;
		~GeneratorMinimaxMoveOrdering() = default;

		void run() {
			nodes = 0;
			hits = 0;

			memo0.clear();
			memo1.clear();
			memo0.reserve(1 << 20);
			memo1.reserve(1 << 20);

			bool firstWins = solve(this->m_initial, 0);

			std::cout << "Calls: " << nodes << "\n";
			std::cout << "Memo hits: " << hits << "\n";
			std::cout << "Wins: " << (firstWins ? "First" : "Second") << " player\n";
		}

	private:
		std::unordered_map<T, uint8_t> memo0, memo1;
		uint64_t nodes = 0;
		uint64_t hits = 0;

		inline bool getMemo(T board, uint8_t turn, bool &out) {
			auto &m = (turn == 0) ? memo0 : memo1;
			auto it = m.find(board);
			if (it == m.end()) return false;
			out = (it->second != 0);
			hits++;
			return true;
		}

		inline void setMemo(T board, uint8_t turn, bool val) {
			auto &m = (turn == 0) ? memo0 : memo1;
			m.emplace(board, uint8_t(val ? 1 : 0));
		}

		bool solve(T current, uint8_t turn) {
			nodes++;

			bool cached;
			if (getMemo(current, turn, cached)) return cached;

			struct Move { T mask; int anchor; };
			std::vector<Move> moves;
			moves.reserve(W * H);

			if (turn == 0) {
				for (uint8_t i = 0; i < H; i++) {
					for (uint8_t j = 0; j < W; j++) {
						if (!GET_BIT(current, W, i, j) && (i + 1 < H) && !GET_BIT(current, W, i + 1, j)) {
							T mask = 0;
							SET_BIT(mask, W, i, j);
							SET_BIT(mask, W, i + 1, j);
							moves.push_back({mask, GET_IDX(W, i, j)});
						}
					}
				}
			} 
			else {
				for (uint8_t i = 0; i < H; i++) {
					for (uint8_t j = 0; j < W; j++) {
						if (!GET_BIT(current, W, i, j) && (j + 1 < W) && !GET_BIT(current, W, i, j + 1)) {
							T mask = 0;
							SET_BIT(mask, W, i, j);
							SET_BIT(mask, W, i, j + 1);
							moves.push_back({mask, GET_IDX(W, i, j)});
						}
					}
				}
			}
				
			if (moves.empty()) {
				setMemo(current, turn, false);
				return false;
			}

			const int centR = H / 2, centC = W / 2;
			auto sortVal = [&](uint16_t idx) -> int {
				int r = idx / W;
				int c = idx % W;
				return std::abs(r - centR) + std::abs(c-centC);
			};
			std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b) {
				return sortVal(a.anchor) < sortVal(b.anchor);
			});
			
			for (const Move& mv : moves) {
				T next = current | mv.mask;
				if (!solve(next, turn^1)) {
					setMemo(current, turn, true);
					return true;
				}
			}

			setMemo(current, turn, false);
			return false;
		}
};
