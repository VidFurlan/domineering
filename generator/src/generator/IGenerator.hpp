#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <vector>

#define GET_IDX(W, i, j) (i * W + j)

#define CLEAR_BIT(x, W, i, j) (x &= ~(T{1} << GET_IDX(W, i, j)))
#define SET_BIT(x, W, i, j) (x |= (T{1} << GET_IDX(W, i, j)))
#define GET_BIT(x, W, i, j) ((x >> GET_IDX(W, i, j)) & 1)

/**
 * @brief Concept for limiting bitboard types to intager types
 */
template <typename T>
concept Bitboard =
	std::is_same_v<T, uint32_t> ||
	std::is_same_v<T, uint64_t> ||
	std::is_same_v<T, __uint128_t>;

/**
 * @brief Interface for solution generation
 */
template <Bitboard T, std::uint8_t W, std::uint8_t H>
class IGenerator {
	public:
		IGenerator() {
			print_bitboard(T{0});
		}

	protected:
		const uint32_t m_size = W * H;

		/**
		 * @brief Merge two bitboards
		 *
		 * @param a 
		 * @param b 
		 * @return T Merged bitboard
		 */
		T merge(const T& a, const T& b) {
			return a | b;
		}

		/**
		 * @brief Split bitboard into components 
		 * for easy processing using DSU
		 *
		 * @param a Bitboard to split
		 * @return std::vector<T> Connected components as bitboards
		 */
		std::vector<T> split(const T& a) {
			std::array<T, m_size> parent(m_size);
			for (uint32_t i = 0; i < m_size; i++) {
				parent[i] = i;
			}

			auto find_parent = [&](auto find_parent, uint32_t v) -> uint32_t {
				if (v == parent[v]) return v;
				return parent[v] = find_parent(parent[v]);
			};

			auto join = [&](uint32_t x, uint32_t y) -> void {
				uint32_t px = find_parent(find_parent, x);
				uint32_t py = find_parent(find_parent, y);
				if (px != py) parent[py] = px;
			};

			for (uint8_t i = 0; i < H; i++) {
				for (uint8_t j = 0; j < W; j++) {
					if (!GET_BIT(a, W, i, j)) continue;
					if (i > 0 && GET_BIT(a, W, i - 1, j)) join(GET_IDX(W, i, j), GET_IDX(W, i - 1, j));
					if (j > 0 && GET_BIT(a, W, i, j - 1)) join(GET_IDX(W, i, j), GET_IDX(W, i, j - 1));
				}
			} 

			std::array<T, m_size> component;
			for (uint32_t i = 0; i < W; i++) {
				for (uint32_t j = 0; j < H; j++) {
					component[parent[i]] |= GET_BIT(a, W, i, j) << GET_IDX(W, i, j);
				}
			}

			std::vector<T> result;
			for (uint32_t i = 0; i < m_size; i++) {
				if (component[i]) result.push_back(component[i]);
			}

			return result;
		}

		void print_bitboard(const T& a) {
			for (uint8_t i = 0; i < H; i++) {
				for (uint8_t j = 0; j < W; j++) {
					std::cout << (GET_BIT(a, W, i, j) ? '#' : '.');
				}
				std::cout << '\n';
			}
			std::cout << '\n';
		}
};
