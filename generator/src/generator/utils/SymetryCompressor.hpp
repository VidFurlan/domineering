#pragma once

#include <algorithm>
#include "generator/Bitboard.hpp"
#include "generator/utils/BitOps.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class SymmetryCompressor {
	private:
		static constexpr int N = W * H;
		int rot180t[N], mirrort[N];
		T rot180(T board) {
			T r = 0;
			while (board) {
				int b = ctzT<T>(board);
				board &= (board - 1);
				r |= (T{1} << rot180t[b]);
			}
			return r;
		}
		T mirror(T board) {
			T r = 0;
			while (board) {
				int b = ctzT<T>(board);
				board &= (board - 1);
				r |= (T{1} << mirrort[b]);
			}
			return r;
		}
	public:
		SymmetryCompressor() {
			for (int i = 0; i < N; i++) {
				int r = i / W, c = i % W;
				rot180t[i] = (H - 1 - r) * W + (W - 1 - c);
				mirrort[i] = r * W + (W - 1 - c);
			}
		}
		T compress(T board) {
			T r = board;
			T r180 = rot180(board);
			r = std::min(r, r180);
			r = std::min(r, mirror(board));
			r = std::min(r, mirror(r180));
			return r;
		}
};
