#pragma once

#include <cstdint>
#include <vector>
#include <type_traits>
#include "generator/AGenerator.hpp"
#include "generator/replace_policies/AlwaysReplace.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H, class ReplacePolicy = AlwaysReplace>
class GeneratorMinimaxTT : public AGenerator<T, W, H> {
	public:
		struct State {
			T board = 0;
			bool turn = 0;      // 0 = vertical/first, 1 = horizontal/second
			uint8_t val = 0;    // 0 = unknown, 1 = lose, 2 = win
			uint32_t depth = 0; // moves from root
		};

	protected:
		using AGenerator<T, W, H>::zobrist;
		using AGenerator<T, W, H>::compressor;
		ReplacePolicy replacePolicy{};

		static constexpr uint64_t TT_SIZE = Zobrist<T, W, H>::STATES;

		std::vector<State> TT0, TT1;
		uint64_t nodes = 0;
		uint64_t hits  = 0;

		static constexpr int N = W * H;
		static constexpr T boardMask =
			(N == int(sizeof(T) * 8)) ? T(~T{0}) : (T{1} << N) - T{1};

		static constexpr T lastColMask() { // prevents horizontal wraparound
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
			else { // __uint128_t
				uint64_t lo = (uint64_t)x;
				if (lo) return __builtin_ctzll(lo);
				uint64_t hi = (uint64_t)(x >> 64);
				return 64 + __builtin_ctzll(hi);
			}
		}

		inline bool getMemo(const State& s, uint64_t hash, bool& out) {
			auto& tt = (s.turn == 0) ? TT0 : TT1;
			size_t idx = hash & (TT_SIZE - 1);

			const State& e = tt[idx];
			if (e.val == 0) return false;
			if (e.board != s.board) return false;

			out = (e.val == 2);
			hits++;
			return true;
		}

		inline void setMemo(const State& s, uint64_t hash, bool win) {
			auto& tt = (s.turn == 0) ? TT0 : TT1;
			size_t idx = hash & (TT_SIZE - 1);

			State neu = s;
			neu.val = win ? 2 : 1;

			State& old = tt[idx];

			if (old.val == 0 || replacePolicy(old, neu)) old = neu;
		}

		bool solve(State s, uint64_t hash) {
			nodes++;

			bool cached;
			if (getMemo(s, hash, cached)) return cached;

			T empty = (~s.board) & boardMask;

			if (s.turn == 0) {
				T anchors = empty & (empty >> W);
				if (!anchors) {
					setMemo(s, hash, false);
					return false;
				}

				while (anchors) {
					int b = ctzT(anchors);
					anchors &= (anchors - 1);

					T a = (T{1} << b);
					T nextBoard = s.board | a | (a << W);

					uint64_t nhash = zobrist.hash(hash, (uint32_t)b);
					nhash = zobrist.hash(nhash, (uint32_t)(b + W));

					State ns;
					ns.board = nextBoard;
					ns.turn  = 1;
					ns.depth = s.depth + 1;

					if (!solve(ns, nhash)) {
						setMemo(s, hash, true);
						return true;
					}
				}
			}
			else {
				T anchors = empty & ~LAST_COL & (empty >> 1);
				if (!anchors) {
					setMemo(s, hash, false);
					return false;
				}

				while (anchors) {
					int b = ctzT(anchors);
					anchors &= (anchors - 1);

					T a = (T{1} << b);
					T nextBoard = s.board | a | (a << 1);

					uint64_t nhash = zobrist.hash(hash, (uint32_t)b);
					nhash = zobrist.hash(nhash, (uint32_t)(b + 1));

					State ns;
					ns.board = nextBoard;
					ns.turn  = 0;
					ns.depth = s.depth + 1;

					if (!solve(ns, nhash)) {
						setMemo(s, hash, true);
						return true;
					}
				}
			}

			setMemo(s, hash, false);
			return false;
		}

	public:
		GeneratorMinimaxTT() = default;
		~GeneratorMinimaxTT() = default;

		void run() override {
			TT0.assign(TT_SIZE, State{});
			TT1.assign(TT_SIZE, State{});
			nodes = 0;
			hits  = 0;

			State root;
			root.board = this->m_initial;
			root.turn  = 0;
			root.depth = 0;

			bool firstWins = solve(root, 0);

			std::cout << "Calls: " << nodes << "\n";
			std::cout << "Memo hits: " << hits << "\n";
			std::cout << "Wins: " << (firstWins ? "First" : "Second") << " player\n";
		}
};
