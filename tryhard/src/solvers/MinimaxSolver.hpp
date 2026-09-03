#pragma once
#include <cstdint>
#include "core/Bitboard.hpp"
#include "core/Board.hpp"
#include "core/MoveGen.hpp"
#include "search/Zobrist.hpp"
#include "cgt/Symmetry.hpp"
#include "eval/Evaluator.hpp"

template <uint8_t W, uint8_t H, class TT_Type>
class MinimaxSolver {
	private:
		using BB = Bitboard<W, H>;
		Zobrist<W, H> zobrist;

	public:
		TT_Type& tt;
		uint64_t nodes = 0;
		uint64_t hits = 0;
		uint64_t splits = 0;
		uint64_t evalterm = 0;

		MinimaxSolver(TT_Type& tt) : tt(tt) {}

		bool solve() {
			Board<W, H> root{};
			nodes = 0;
			hits = 0;
			auto res = solve_recursive(root, 0) == 0;
			std::cout << "nodes:  " << nodes  << "\n";
			std::cout << "hits:   " << hits   << "\n";
			std::cout << "splits: " << splits << "\n";
			std::cout << "eval:   " << nodes-evalterm << "\n";
			return res;
		}

	private:
		uint8_t solve_recursive(const Board<W, H>& board, uint8_t depth) {
			nodes++;
			//board.print();
			auto spt = Splitter<W, H>::split(board.occupied);
			if (spt.size() > 1) splits++;

			BB empty = ~board.occupied & MoveGen<W, H>::get_full_mask();

			int eval = Evaluator<W, H>::check_guaranteed_win(empty, board.turn);
			if (eval == 1) return board.turn;
			if (eval == -1) return board.turn^1;
			evalterm++;

			BB mask = Symmetry<W, H>::get_canonical(empty);
			uint64_t hash = zobrist.full_hash(mask);

			TTEntry<W, H> entry;
			if (tt.get(hash, mask, entry)) {
				hits++;
				return entry.value - 1;
			}

			BB all_anchors = MoveGen<W, H>::get_anchors(board);
			if (!all_anchors) {
				tt.store(hash, {mask, depth, static_cast<uint8_t>((board.turn ^ 1) + 1)});
				return board.turn ^ 1; 
			}

			for (BB ring : MoveGen<W, H>::rings) {
				BB anchors = all_anchors & ring;
				while (anchors) {
					int bit = ctz(anchors);
					BB move = (BB{1} << bit);
					if (board.turn == 0) move = move | (BB{1} << (bit + W));
					else move = move | (BB{1} << (bit + 1));

					Board<W, H> next = board;
					next.occupied = next.occupied | move;
					next.turn = board.turn ^ 1;

					uint8_t result = solve_recursive(next, depth + 1);
					if (result == board.turn) {
						tt.store(hash, {mask, depth, static_cast<uint8_t>(board.turn + 1)});
						return board.turn;
					}

					anchors = anchors & (anchors - 1);
				}
			}

			tt.store(hash, {mask, depth, static_cast<uint8_t>((board.turn ^ 1) + 1)});
			return board.turn ^ 1;
		}
};
