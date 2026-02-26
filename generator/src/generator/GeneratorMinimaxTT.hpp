#pragma once

#include <cstdint>
#include <iostream>
#include <stack>
#include <tuple>
#include "generator/AGenerator.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class GeneratorMinimaxTT : public AGenerator<T, W, H> {
	struct State {
		T board;
		uint8_t val; // 0-unchecked, 1-lose, 2-win
	};
	using AGenerator<T,W,H>::zobrist;

	public:
		GeneratorMinimaxTT() = default;
		~GeneratorMinimaxTT() = default;

		void run() {
			TT0.resize(TT_SIZE);
			TT1.resize(TT_SIZE);
			nodes = 0;
			hits = 0;

			bool firstWins = solve(this->m_initial, 0, 0);

			std::cout << "Calls: " << nodes << "\n";
			std::cout << "Memo hits: " << hits << "\n";
			std::cout << "Wins: " << (firstWins ? "First" : "Second") << " player\n";
		}

	private:


		static constexpr uint64_t TT_SIZE = Zobrist<T, W, H>::STATES;
		std::vector<State> TT0, TT1;
		uint64_t nodes = 0;
		uint64_t hits = 0;

		inline bool shouldReplace(T oldb, T newb) {
			return false;
		}

		inline bool getMemo(T board, uint64_t hash, uint8_t turn, bool &out) {
			auto &tt = (turn == 0) ? TT0 : TT1;
			if (tt[hash].board != board || tt[hash].val == 0) return false;
			out = tt[hash].first;
			hits++;
			return true;
		}

		inline void setMemo(T board, uint64_t hash, uint8_t turn, bool val) {
			auto &tt = (turn == 0) ? TT0 : TT1;
			if (shouldReplace(tt[hash].second, board)) tt[hash] = {val ? 1 : 0, board};
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

		bool solve(T current, uint64_t hash, uint8_t turn) {
			nodes++;

			bool cached;
			if (getMemo(current, hash, turn, cached)) return cached;

			T empty = (~current) & boardMask;

			if (turn == 0) {
				T anchors = empty & (empty >> W);
				if (!anchors) {
					setMemo(current, hash, turn, false);
					return false;
				}
				
				while (anchors) {
					int b = ctzT(anchors);
					anchors &= anchors - 1;
					T a = T{1} << b;
					T next = current | a | (a << W);
					uint64_t nhash = zobrist.hash(hash, b);
					nhash = zobrist.hash(nhash, b+W);
					if (!solve(next, nhash, 1)) {
						setMemo(current, nhash, turn, true);
						return true;
					}
				}
			} 
			else {
				T anchors = empty & ~LAST_COL & (empty >> 1);
				if (!anchors) {
					setMemo(current, hash, turn, false);
					return false;
				}
				
				while (anchors) {
					int b = ctzT(anchors);
					anchors &= anchors - 1;
					T a = T{1} << b;
					T next = current | a | (a << 1);
					uint64_t nhash = zobrist.hash(hash, b);
					nhash = zobrist.hash(nhash, b+1);
					if (!solve(next, nhash, 1)) {
						setMemo(current, nhash, turn, true);
						return true;
					}
				}
			}

			setMemo(current, hash, turn, false);
			return false;
		}
};
