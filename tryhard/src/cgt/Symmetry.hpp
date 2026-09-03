#pragma once
#include <cstdint>
#include <algorithm>
#include "core/Bitboard.hpp"

template <uint8_t W, uint8_t H>
struct Symmetry {
	using BB = Bitboard<W, H>;
	
	static BB to_origin(BB mask) {
		if (!mask) return mask;
		int min_r = H;
		int min_c = W;
		BB temp = mask;
		while (temp) {
			int bit = ctz(temp);
			int r = bit / W;
			int c = bit % W;
			if (r < min_r) min_r = r;
			if (c < min_c) min_c = c;
			temp &= temp - 1;
		}
		int shift = min_r * W + min_c; // oohhhh gracciasss gemini
		return mask >> shift;
	}
	
	static BB flip_x(BB mask) {
		BB res = 0;
		while (mask) {
			int bit = ctz(mask);
			int r = bit / W;
			int c = bit % W;
			int flip = r * W + W - 1 - c;
			res |= BB{1} << flip;
			mask &= mask - 1;
		}
		return res;
	}
	
	static BB flip_y(BB mask) {
		BB res = 0;
		while (mask) {
			int bit = ctz(mask);
			int r = bit / W;
			int c = bit % W;
			int flip = (H - 1 - r) * W + c;
			res |= BB{1} << flip;
			mask &= mask - 1;
		}
		return res;
	}

	static BB get_canonical(BB mask) {
		if (!mask) return mask;
		mask = to_origin(mask);
		BB fx = to_origin(flip_x(mask));
		BB fxy = to_origin(flip_y(fx));
		BB fy = to_origin(flip_y(mask));
		if (fx < mask) mask = fx;
		if (fxy < mask) mask = fxy;
		if (fy < mask) mask = fy;
		return mask;
	}
};
