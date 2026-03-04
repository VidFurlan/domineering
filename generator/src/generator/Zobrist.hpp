#pragma once

#include <cstdint>
#include <array>
#include "generator/Bitboard.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H>
class Zobrist {
	public:
		static constexpr int POWER = 26;
		static constexpr int STATES = 1<<POWER;
	private:
		std::array<uint64_t, W*H> bitValue;
		static uint64_t splitmix64(uint64_t& x) {
			uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
			z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
			z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
			return z ^ (z >> 31);
		}
	public:
		Zobrist(uint64_t seed = 42424200) {
			for (int i = 0; i < W*H; i++) {
				bitValue[i] = splitmix64(seed);
			}
		}
		uint64_t hash(uint64_t old, uint32_t bit) {
			return old ^ bitValue[bit];
		}
};
