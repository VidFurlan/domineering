#pragma once
#include <vector>
#include "core/Bitboard.hpp"
#include "core/MoveGen.hpp"

template <uint8_t W, uint8_t H>
struct Splitter {
	using BB = Bitboard<W, H>;
	static constexpr BB get_first_col_mask() {
		BB m{};
		for (int r = 0; r < H; ++r) m |= (BB{1} << (r * W));
		return m;
	}

	//https://stackoverflow.com/questions/21470531/flood-fill-with-bitwise-operations
	static std::vector<BB> split(BB empty) { // return masks
		std::vector<BB> comps;
		comps.reserve(8); 
		constexpr BB NOT_LAST = ~MoveGen<W, H>::get_last_col_mask();
		constexpr BB NOT_FIRST = ~get_first_col_mask();

		while (empty) {
			int start = ctz(empty);
			BB comp = BB{1} << start;
			while (true) {
				BB next = comp;
				next |= ((comp << 1) & NOT_FIRST);
				next |= ((comp >> 1) & NOT_LAST);
				next |= comp << W;
				next |= comp >> W;
				next &= empty;
				if (next == comp) break;
				comp = next;
			}
			comps.push_back(comp);
			empty = empty & ~comp;
		}
		return comps;
	}
};
