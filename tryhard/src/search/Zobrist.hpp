#pragma once
#include <cstdint>
#include <array>
#include "core/Bitboard.hpp"

//https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
class Splitmix64 {
	private:
		uint64_t state;

	public:
		void seed(const uint64_t seed) {
			state = seed;
		}

		uint64_t next_int() {
			uint64_t z = ( state += 0x9e3779b97f4a7c15 );
			z = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9;
			z = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111eb;
			return z ^ ( z >> 31 );
		}
};

// https://www.chessprogramming.org/Zobrist_Hashing
template <uint8_t W, uint8_t H>
class Zobrist {
	public:
		using BB = Bitboard<W, H>;
		static constexpr int N = W*H;

	private:
		std::array<uint64_t, N> bit_values;
		Splitmix64 splitmix;		

	public:
		explicit Zobrist(uint64_t seed = 26072007ull) {
			splitmix.seed(seed);
			for (int i = 0; i < N; i++) bit_values[i] = splitmix.next_int();
		}

		uint64_t full_hash(BB mask) {
			uint64_t hash = 0;
			while (mask) {
				int bit = ctz(mask);
				hash ^= bit_values[bit];
				mask &= mask - 1;
			}
			return hash;
		}
		
		uint64_t increment_hash(uint64_t hash, int bit1, int bit2) {
			return hash ^ bit_values[bit1] ^ bit_values[bit2];
		}
};

