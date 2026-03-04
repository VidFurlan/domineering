#pragma once

#include <cstdint>
#include "generator/Bitboard.hpp"

template <Bitboard T>
int ctzT(const T& x) {
	if constexpr (std::is_same_v<T, uint32_t>) return __builtin_ctz(x);
	else if constexpr (std::is_same_v<T, uint64_t>) return __builtin_ctzll(x);
	else { // __uint128_t
		uint64_t lo = (uint64_t)x;
		if (lo) return __builtin_ctzll(lo);
		uint64_t hi = (uint64_t)(x >> 64);
		return 64 + __builtin_ctzll(hi);
	}
}
