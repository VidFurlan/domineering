#pragma once

#include <type_traits>
#include <cstdint>

/**
 * @brief Concept for limiting bitboard types to intager types
 */
template <typename T>
concept Bitboard =
	std::is_same_v<T, uint32_t> ||
	std::is_same_v<T, uint64_t> ||
	std::is_same_v<T, __uint128_t>;
