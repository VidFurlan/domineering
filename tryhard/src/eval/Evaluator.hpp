#pragma once
#include <cstdint>
#include "core/Bitboard.hpp"
#include "core/MoveGen.hpp"

template <uint8_t W, uint8_t H>
struct Evaluator {
	using BB = Bitboard<W, H>;

	static int check_guaranteed_win(const BB& empty, bool turn) {
		BB v = empty & (empty >> W);
		BB h = empty & (empty >> 1) & ~MoveGen<W, H>::get_last_col_mask();
		int v_cnt = popcount(v);
		int h_cnt = popcount(h);

		BB h_occup = h | (h << 1);
		BB v_safe = v & ~h_occup & ~(h_occup >> W);
		int v_cnt_safe = popcount(v_safe);
		
		BB v_occup = v | (v << W);
		BB h_safe = h & ~v_occup & ~(v_occup >> 1);
		int h_cnt_safe = popcount(h_safe);
		
		if (turn == 0) {
			if (v_cnt_safe > h_cnt) return 1;
			if (v_cnt == 0 || v_cnt <= h_cnt_safe) return -1;
		}
		else {
			if (h_cnt_safe > v_cnt) return 1;
			if (h_cnt == 0 || h_cnt <= v_cnt_safe) return -1;
		}
		return 0;
	}
};
