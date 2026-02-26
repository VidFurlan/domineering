#pragma once

#include <cstdint>
#include <iostream>
#include <stack>
#include <tuple>
#include "generator/AGenerator.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class GeneratorMinimaxBitboard : public AGenerator<T, W, H> {
	public:
		GeneratorMinimaxBitboard() = default;
		~GeneratorMinimaxBitboard() = default;

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

		static constexpr int N = W * H;

		static constexpr T boardMask = (N == int(sizeof(T)*8)) ? T(~T{0}) : (T{1} << N) - T{1};

		static constexpr T lastColMask() { // prepreci wraparound nove domine
			T m = 0;
			for (int r = 0; r < H; ++r) {
				m |= (T{1} << (r * W + (W - 1)));
			}
			return m;
		}
		static constexpr T LAST_COL = lastColMask();

		static inline int ctzT(T x) {
			if constexpr (std::is_same_v<T, uint32_t>) return __builtin_ctz(x);
			else if constexpr (std::is_same_v<T, uint64_t>) return __builtin_ctzll(x);
			else { // int128
				uint64_t lo = (uint64_t)x;
				if (lo) return __builtin_ctzll(lo);
				uint64_t hi = (uint64_t)(x >> 64);
				return 64 + __builtin_ctzll(hi);
			}
		}

		bool solve(T current, uint8_t turn) {
			nodes++;

			bool cached;
			if (getMemo(current, turn, cached)) return cached;

			T empty = (~current) & boardMask;

			if (turn == 0) {
				T anchors = empty & (empty >> W);
				if (!anchors) {
					setMemo(current, turn, false);
					return false;
				}
				
				while (anchors) {
					int b = ctzT(anchors);
					anchors &= anchors - 1;
					T a = T{1} << b;
					T next = current | a | (a << W);
					if (!solve(next, 1)) {
						setMemo(current, turn, true);
						return true;
					}
				}
			} 
			else {
				T anchors = empty & ~LAST_COL & (empty >> 1);
				if (!anchors) {
					setMemo(current, turn, false);
					return false;
				}
				
				while (anchors) {
					int b = ctzT(anchors);
					anchors &= anchors - 1;
					T a = T{1} << b;
					T next = current | a | (a << 1);
					if (!solve(next, 1)) {
						setMemo(current, turn, true);
						return true;
					}
				}
			}

			setMemo(current, turn, false);
			return false;
		}
};
