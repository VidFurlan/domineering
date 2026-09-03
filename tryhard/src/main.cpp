#include <iostream>
#include <vector>

#include "core/Bitboard.hpp"
#include "core/Board.hpp"
#include "core/MoveGen.hpp"
#include "cgt/Splitter.hpp"
#include "cgt/TranspositionTable.hpp"

#include "solvers/MinimaxSolver.hpp"

static int get_int_arg(int argc, char** argv, const std::string& key, int def) {
	for (int i = 1; i + 1 < argc; i++) {
		if (argv[i] == key)
			return std::atoi(argv[i + 1]);
	}
	return def;
}

static uint64_t get_u64_arg(int argc, char** argv, const std::string& key, uint64_t def) {
	for (int i = 1; i + 1 < argc; i++) {
		if (argv[i] == key)
			return std::strtoull(argv[i + 1], nullptr, 0);
	}
	return def;
}

static std::string get_str_arg(int argc, char** argv, const std::string& key, const std::string& def = "") {
	for (int i = 1; i + 1 < argc; i++) {
		if (argv[i] == key) return argv[i + 1];
	}
	return def;
}

static bool has_flag(int argc, char** argv, const std::string& key) {
	for (int i = 1; i < argc; i++) {
		if (argv[i] == key) return true;
	}
	return false;
}

template <uint8_t W, uint8_t H, class TT_Type>
void choose_solver(const std::string& algo, TT_Type& tt) {
	if (algo == "minimax") {
		MinimaxSolver<W, H, TT_Type> solver(tt);
		bool win_p1 = solver.solve();
		std::cout << "win: p" << "21"[win_p1] << "\n";
	} 
	else if (algo == "cgt") {} 
	else std::cerr << "Bad algo param: " << algo << "\n";
}

template <uint8_t W, uint8_t H>
void choose_tt(const std::string& tt_policy, size_t tt_size_mb, const std::string& algo) {
	if (tt_policy == "depth") {
		TranspositionTable<W, H, DepthReplace> tt(tt_size_mb);
		choose_solver<W, H>(algo, tt);
	} 
	else if (tt_policy == "depth2") {
		TranspositionTable<W, H, Depth2Replace> tt(tt_size_mb);
		choose_solver<W, H>(algo, tt);
	} 
	else if (tt_policy == "always") {
		TranspositionTable<W, H, AlwaysReplace> tt(tt_size_mb);
		choose_solver<W, H>(algo, tt);
	} 
	else {
		std::cerr << "Bad TT param: " << tt_policy << "\n";
	}
}

template <uint8_t W, uint8_t H>
int run_solver(const std::string& algo, const std::string& tt_policy, size_t tt_size) {
	choose_tt<W, H>(tt_policy, tt_size, algo);
	return 0;
}

int main(int argc, char** argv) {
	int w = get_int_arg(argc, argv, "-w", 5);
	int h = get_int_arg(argc, argv, "-h", 5);
	int memory = get_int_arg(argc, argv, "--h", 4000); // Default 2GB
	std::string algo = get_str_arg(argc, argv, "--algo", "minimax");
	std::string tt = get_str_arg(argc, argv, "--tt", "depth2");
#define CASE(W,H) if (w == (W) && h == (H)) return run_solver<(W),(H)>(algo, tt, memory);
	CASE(1, 1); CASE(1, 2); CASE(1, 3); CASE(1, 4); CASE(1, 5); CASE(1, 6); CASE(1, 7); CASE(1, 8);
	CASE(2, 1); CASE(2, 2); CASE(2, 3); CASE(2, 4); CASE(2, 5); CASE(2, 6); CASE(2, 7); CASE(2, 8);
	CASE(3, 1); CASE(3, 2); CASE(3, 3); CASE(3, 4); CASE(3, 5); CASE(3, 6); CASE(3, 7); CASE(3, 8);
	CASE(4, 1); CASE(4, 2); CASE(4, 3); CASE(4, 4); CASE(4, 5); CASE(4, 6); CASE(4, 7); CASE(4, 8);
	CASE(5, 1); CASE(5, 2); CASE(5, 3); CASE(5, 4); CASE(5, 5); CASE(5, 6); CASE(5, 7); CASE(5, 8);
	CASE(6, 1); CASE(6, 2); CASE(6, 3); CASE(6, 4); CASE(6, 5); CASE(6, 6); CASE(6, 7); CASE(6, 8);
	CASE(7, 1); CASE(7, 2); CASE(7, 3); CASE(7, 4); CASE(7, 5); CASE(7, 6); CASE(7, 7); CASE(7, 8);
	CASE(8, 1); CASE(8, 2); CASE(8, 3); CASE(8, 4); CASE(8, 5); CASE(8, 6); CASE(8, 7); CASE(8, 8);
	//CASE(X, 1); CASE(X, 2); CASE(X, 3); CASE(X, 4); CASE(X, 5); CASE(X, 6); CASE(X, 7); CASE(X, 8);
#undef CASE
	std::cerr << "Bad size " << w << "x" << h << "\n";
	return 1;
}
