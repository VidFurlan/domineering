#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <chrono>
#include "tree/GameTree.hpp"
#include "generator/Bitboard.hpp"

#define GET_IDX(W, i, j) ((i) * (W) + (j))

#define CLEAR_BIT(x, W, i, j) ((x) &= ~(T{1} << GET_IDX(W, i, j)))
#define SET_BIT(x, W, i, j) ((x) |= (T{1} << GET_IDX(W, i, j)))
#define GET_BIT(x, W, i, j) (((x) >> GET_IDX(W, i, j)) & T{1})

/**
 * @brief Interface for solution generation
 */
template <Bitboard T, std::uint8_t W, std::uint8_t H>
class AGenerator {
	public:
		AGenerator(const T& initial = 0) : m_initial(initial) {}
		virtual void run() = 0;
		void benchmark(uint32_t iterations = 1) {
			uint64_t total_ms = 0;
			for (uint32_t i = 0; i < iterations; i++) {
				m_tree = GameTree<T>();

				std::cout << "Iteration " << (i + 1) << ":\n";
				auto start = std::chrono::high_resolution_clock::now();

				run();

				auto end = std::chrono::high_resolution_clock::now();
				auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
				total_ms += duration_ms.count();

				//std::cout << "Total states: " << m_tree.size() << '\n';
				//std::cout << "Time taken: " << duration_ms.count() << "ms\n";
				//std::cout << "\n";
			}
			std::cout << "Average time over " << iterations << " iterations: " << (total_ms / iterations) << "ms\n\n";
			std::cout << "----------------------------------------\n\n";
		}

	protected:
		static constexpr uint32_t c_size = W * H;
		GameTree<T> m_tree;
		const T m_initial;

		T merge(const T& a, const T& b);
		std::vector<T> split(const T& a);

		void print(const T& a);
};


/**
 * @brief Merge two bitboards
 *
 * @param a 
 * @param b 
 * @return T Merged bitboard
 */
template <Bitboard T, std::uint8_t W, std::uint8_t H>
inline T AGenerator<T, W, H>::merge(const T& a, const T& b) {
	return a | b;
}

/**
 * @brief Split bitboard into components 
 * for easy processing using DSU
 *
 * @param a Bitboard to split
 * @return std::vector<T> Connected components as bitboards
 */
template <Bitboard T, std::uint8_t W, std::uint8_t H>
inline std::vector<T> AGenerator<T, W, H>::split(const T& a) {
	std::array<T, c_size> parent;
	for (uint32_t i = 0; i < c_size; i++) {
		parent[i] = i;
	}

	auto find_parent = [&](auto find_parent, uint32_t v) -> uint32_t {
		if (v == parent[v]) return v;
		return parent[v] = find_parent(find_parent, parent[v]);
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

	std::array<T, c_size> component;
	for (uint32_t i = 0; i < W; i++) {
		for (uint32_t j = 0; j < H; j++) {
			component[find_parent(find_parent, GET_IDX(W, i, j))] |= GET_BIT(a, W, i, j) << GET_IDX(W, i, j);
		}
	}

	std::vector<T> result;
	for (uint32_t i = 0; i < c_size; i++) {
		if (component[i]) result.push_back(component[i]);
	}

	return result;
}

/**
 * @brief Print bitboard to console
 *
 * @param a 
 */
template <Bitboard T, std::uint8_t W, std::uint8_t H>
inline void AGenerator<T, W, H>::print(const T& a) {
	for (uint8_t i = 0; i < H; i++) {
		for (uint8_t j = 0; j < W; j++) {
			std::cout << (GET_BIT(a, W, i, j) ? '#' : '.');
		}
		std::cout << '\n';
	}
	std::cout << '\n';
}
