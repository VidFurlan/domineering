#pragma once
#include <cstdint>
#include <iostream>
#include "Bitboard.hpp"

template <uint8_t W, uint8_t H>
struct Board {
	using BB = Bitboard<W, H>;
	BB occupied{};
	bool turn = 0;

	Board() {}
	Board(BB init) { occupied = init; }

	inline bool operator==(const Board& oth) const {
		return turn == oth.turn && occupied == oth.occupied;
	}

	void print() const {
		std::cout << "Board " << (int)W << "x" << (int)H << ":\n";
		for (int r = 0; r < H; r++) {
			for (int c = 0; c < W; c++) {
				std::cout << "[" << ".#"[(occupied >> (r * W + c)) & 1] << "]";
			} 
			std::cout << "\n";
		} 
		std::cout << "Turn: " << "VH"[turn] << "\n\n";
	}
};
