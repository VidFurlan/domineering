#pragma once

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "generator/GeneratorMinimaxTTMoveOrderingNoComp.hpp"
// def used in main to precompile main
#define DEMO_CASE(W,H) \
	if (w == (W) && h == (H)) { \
		using T = BB<W, H>; \
		auto res = DemoInteractor<T, W, H>::solvePosition((T)board_arg, (bool)turn, want_best_move); \
		if (json) std::cout << DemoInteractor<T, W, H>::toJson(res) << "\n"; \
		else { \
			std::cout << "Win for side-to-move: " << (res.win ? "true" : "false") << "\n"; \
			if (res.bestMove.has_value()) { \
				std::cout << "Best move: (" << res.bestMove->r << ", " << res.bestMove->c << ", " << res.bestMove->dir << ")\n"; \
			} else { \
				std::cout << "Best move: none\n"; \
			} \
			std::cout << "Nodes: " << res.nodes << "\n"; \
			std::cout << "Hits: " << res.hits << "\n"; \
		} \
		return 0; \
	}

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class DemoInteractor {
	public:
		using Solver = GeneratorMinimaxTTMoveOrderingNoComp<T, W, H>;
		using State = typename Solver::State;

		struct Move {
			int r = 0;
			int c = 0;
			char dir = 'V';
		};

		struct Result {
			bool win = false;
			std::optional<Move> bestMove;
			uint64_t nodes = 0;
			uint64_t hits = 0;
			uint64_t ms = 0;
		};

		static Result solvePosition(T board, bool turn, bool wantBestMove) {
			Solver probe;
			probe.reset_tables();

			State root{};
			root.board = board;
			root.turn = turn;
			root.depth = 0;
			root.val = 0;

			Result res{};

			res.win = probe.solve_subtree(root, 0);
			res.nodes = probe.get_nodes();
			res.hits = probe.get_hits();

			if (wantBestMove) res.bestMove = findBestMove(board, turn);
			return res;
		}

		static std::string toJson(const Result& r) {
			std::ostringstream oss;
			oss << "{";
			oss << "\"win\":" << (r.win ? "true" : "false") << ",";
			if (r.bestMove.has_value()) {
				oss << "\"bestMove\":{";
				oss << "\"r\":" << r.bestMove->r << ",";
				oss << "\"c\":" << r.bestMove->c << ",";
				oss << "\"dir\":\"" << r.bestMove->dir << "\"";
				oss << "},";
			} else {
				oss << "\"bestMove\":null,";
			}
			oss << "\"nodes\":" << r.nodes << ",";
			oss << "\"hits\":" << r.hits << ",";
			oss << "\"ms\":" << r.ms;
			oss << "}";
			return oss.str();
		}

	private:
		static constexpr int N = W * H;
		static constexpr T boardMask =
			(N == int(sizeof(T) * 8)) ? T(~T{0}) : (T{1} << N) - T{1};

		static constexpr T lastColMask() {
			T m = 0;
			for (int r = 0; r < H; ++r) {
				m |= (T{1} << (r * W + (W - 1)));
			}
			return m;
		}
		static constexpr T LAST_COL = lastColMask();

		static inline int ctzT_local(T x) {
			if constexpr (std::is_same_v<T, uint32_t>) return __builtin_ctz(x);
			else if constexpr (std::is_same_v<T, uint64_t>) return __builtin_ctzll(x);
			else {
				uint64_t lo = (uint64_t)x;
				if (lo) return __builtin_ctzll(lo);
				uint64_t hi = (uint64_t)(x >> 64);
				return 64 + __builtin_ctzll(hi);
			}
		}

		static std::optional<Move> findBestMove(T board, bool turn) {
			T empty = (~board) & boardMask;
			std::optional<Move> fallback;
			if (!turn) {
				T anchors = empty & (empty >> W);
				while (anchors) {
					int b = ctzT_local(anchors);
					anchors &= (anchors - 1);

					Move mv{ b / W, b % W, 'V' };
					if (!fallback.has_value()) fallback = mv;

					T a = (T{1} << b);
					T nextBoard = board | a | (a << W);

					Solver childSolver;
					childSolver.reset_tables();

					State child{};
					child.board = nextBoard;
					child.turn = 1;
					child.depth = 1;
					child.val = 0;

					bool oppWins = childSolver.solve_subtree(child, 0);
					if (!oppWins) return mv;
				}
			}
			else {
				T anchors = empty & ~LAST_COL & (empty >> 1);
				while (anchors) {
					int b = ctzT_local(anchors);
					anchors &= (anchors - 1);

					Move mv{ b / W, b % W, 'H' };
					if (!fallback.has_value()) fallback = mv;

					T a = (T{1} << b);
					T nextBoard = board | a | (a << 1);

					Solver childSolver;
					childSolver.reset_tables();

					State child{};
					child.board = nextBoard;
					child.turn = 0;
					child.depth = 1;
					child.val = 0;

					bool oppWins = childSolver.solve_subtree(child, 0);
					if (!oppWins) return mv;
				}
			}
			return fallback;
		}
};
