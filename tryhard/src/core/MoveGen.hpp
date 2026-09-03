#pragma once
#include "Board.hpp"
#include "Bitboard.hpp"

template <uint8_t W, uint8_t H>
struct MoveGen {
	using BB = Bitboard<W, H>;

	static constexpr int RINGS = (W + H) / 2 + 1;

	static constexpr BB get_full_mask() {
		constexpr int N = W * H;
		return (N == sizeof(BB) * 8) ? BB(~BB{0}) : (BB{1} << N) - BB{1};
	}

	static constexpr BB get_last_col_mask() {
		BB m = 0;
		for (int r = 0; r < H; ++r) m = m | (BB{1} << (r * W + (W - 1)));
		return m;
	}

	static inline BB get_anchors(const Board<W, H>& board) {
		BB empty = ~board.occupied & get_full_mask();
		BB v = empty & (empty >> W);
		BB h = empty & (empty >> 1) & ~get_last_col_mask();
		if (board.turn == 0) {
			BB h_occup = h | (h << 1);
			BB safe = v & ~h_occup & ~(h_occup >> W);
			BB both = v ^ safe;
			return both ? both : safe;
		}
		else {
			BB v_occup = v | (v << W);
			BB safe = h & ~v_occup & ~(v_occup >> 1);
			BB both = h ^ safe;
			return both ? both : safe;
		}
	}

	static constexpr std::array<BB, RINGS> compute_center_rings() {
		std::array<BB, RINGS> rings{};
		for (int r = 0; r < H; r++) {
			for (int c = 0; c < W; c++) {
				int dr = std::abs(r * 2 - (H - 1));
				int dc = std::abs(c * 2 - (W - 1));
				int d = std::max(dr, dc) / 2;
				rings[d] = rings[d] | (BB{1} << (r * W + c));
			}
		}
		return rings;
	}
	static constexpr std::array<BB, RINGS> rings = compute_center_rings();
};
