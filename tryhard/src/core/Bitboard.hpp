#pragma once
#include <cstdint>
#include <type_traits>

template <uint8_t W, uint8_t H>
struct BitboardSelector {
	static constexpr int BITS = W * H;
	using Type = std::conditional_t<
		(BITS <= 64), uint64_t,
		unsigned __int128
	>;
};

template <uint8_t W, uint8_t H>
using Bitboard = typename BitboardSelector<W, H>::Type;

template <typename T>
inline int ctz(const T& x) {
	if constexpr (std::is_same_v<T, uint64_t>) {
		return __builtin_ctzll(x);
	} 
	else if constexpr (std::is_same_v<T, unsigned __int128>) {
		uint64_t low = (uint64_t)x;
		if (low) return __builtin_ctzll(low);
		return 64 + __builtin_ctzll((uint64_t)(x >> 64));
	} 
	else {
	}
}

template <typename T>
inline int popcount(const T& x) {
	if constexpr (std::is_same_v<T, uint64_t>) {
		return __builtin_popcountll(x);
	} 
	else if constexpr (std::is_same_v<T, unsigned __int128>) {
		return __builtin_popcountll((uint64_t)x) + __builtin_popcountll((uint64_t)(x >> 64));
	} 
	else {
	}
}
