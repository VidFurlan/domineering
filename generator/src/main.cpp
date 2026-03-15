#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "generator/AGenerator.hpp"
#include "generator/GeneratorMinimaxBruteforce.hpp"
#include "generator/GeneratorMinimaxMemo.hpp"
#include "generator/GeneratorMinimaxMemoNoGraph.hpp"
#include "generator/GeneratorMinimaxMoveOrdering.hpp"
#include "generator/GeneratorMinimaxBitboard.hpp"
#include "generator/GeneratorMinimaxBitboardMoveOrdering.hpp"
#include "generator/GeneratorMinimaxTT.hpp"
#include "generator/GeneratorMinimaxTTMoveOrdering.hpp"
#include "generator/GeneratorMinimaxTTMoveOrderingNoComp.hpp"
#include "generator/GeneratorMinimaxTTMoveOrderingDualTT.hpp"
#include "generator/GeneratorMinimaxRootParallel.hpp"

#include "generator/replace_policies/AlwaysReplace.hpp"
#include "generator/replace_policies/DepthReplace.hpp"

static int get_int_arg(int argc, char** argv, const std::string& key, int def) {
	for (int i = 1; i + 1 < argc; ++i) {
		if (argv[i] == key)
			return std::atoi(argv[i + 1]);
	}
	return def;
}

static std::string get_str_arg(int argc, char** argv, const std::string& key, const std::string& def = "") {
	for (int i = 1; i + 1 < argc; ++i) {
		if (argv[i] == key) return argv[i + 1];
	}
	return def;
}

static bool has_flag(int argc, char** argv, const std::string& key) {
	for (int i = 1; i < argc; ++i) {
		if (argv[i] == key) return true;
	}
	return false;
}

template <uint8_t W, uint8_t H>
using BB = std::conditional_t<(W * H <= 32), uint32_t, uint64_t>;

template <uint8_t W, uint8_t H>
int run_bench_auto(int runs, const std::string& out, const std::string& fmt, std::string algo, int threads = 1) {
	AGenerator<BB<W, H>, W, H> *gen;
	if (algo == "bf") gen = new GeneratorMinimaxBruteforce<BB<W, H>, W, H>(); 
	else if (algo == "memo") gen = new GeneratorMinimaxMemo<BB<W, H>, W, H>(); 
	else if (algo == "memo-nograph") gen = new GeneratorMinimaxMemoNoGraph<BB<W, H>, W, H>(); 
	else if (algo == "mo") gen = new GeneratorMinimaxMoveOrdering<BB<W, H>, W, H>(); 
	else if (algo == "bb") gen = new GeneratorMinimaxBitboard<BB<W, H>, W, H>(); 
	else if (algo == "bbmo") gen = new GeneratorMinimaxBitboardMoveOrdering<BB<W, H>, W, H>(); 
	else if (algo == "tt") gen = new GeneratorMinimaxTT<BB<W, H>, W, H>(); 
	else if (algo == "ttmo") gen = new GeneratorMinimaxTTMoveOrdering<BB<W, H>, W, H>(); 
	else if (algo == "ttmonc") gen = new GeneratorMinimaxTTMoveOrderingNoComp<BB<W, H>, W, H>(); 
	else if (algo == "ttmodd") gen = new GeneratorMinimaxTTMoveOrderingDualTT<BB<W, H>, W, H>();
	else if (algo == "ttmorp") gen = new GeneratorMinimaxRootParallel<BB<W, H>, W, H, GeneratorMinimaxTTMoveOrderingNoComp<BB<W, H>, W, H>>(threads);
	else gen = new GeneratorMinimaxTTMoveOrderingNoComp<BB<W, H>, W, H>();
	gen->benchmark(runs);

	if (!out.empty()) {
		gen->saveTree(out, fmt);
	}
	return 0;
}

int main(int argc, char** argv) {
	bool help = has_flag(argc, argv, "--help");
	if (help) {
		std::cout << "Usage: " << argv[0] << " [--algo <algo>] [--w <width>] [--h <height>] [--runs <iterations>] [--save-graph <path>] [--format <edges|json>] [--threads <number of threads>]\n";
		std::cout << "--algo: \n";
		std::cout << "\tbf - brute-force\n";
		std::cout << "\tmemo - memoization\n";
		std::cout << "\tmemo-nograph - memoization without graph construction\n";
		std::cout << "\tmo - memoization with move ordering\n";
		std::cout << "\tbb - bitboard\n";
		std::cout << "\tbbmo - bitboard with move ordering\n";
		std::cout << "\ttt - transposition table\n";
		std::cout << "\tttmo - transposition table with move ordering\n";
		std::cout << "\tttmonc - transposition table with move ordering, no compression\n";
		std::cout << "\tttmodd - transposition table with move ordering, dual TT\n";
		std::cout << "\tttmorp - root-parallel transposition table with move ordering, no compression\n";
		std::cout << std::flush;
		return 0;
	}
	int w = get_int_arg(argc, argv, "--w", 5);
	int h = get_int_arg(argc, argv, "--h", 5);
	int runs = get_int_arg(argc, argv, "--runs", 1);
	int threads = get_int_arg(argc, argv, "--threads", 1);

	std::string out = get_str_arg(argc, argv, "--save-graph", "");
	std::string fmt = get_str_arg(argc, argv, "--format", "edges");
	std::string algo = get_str_arg(argc, argv, "--algo", "ttmonc");

	// clang-format off
#define CASE(W,H) if (w == (W) && h == (H)) return run_bench_auto<(W),(H)>(runs, out, fmt, algo, threads);
	CASE(1,1) CASE(1,2) CASE(1,3) CASE(1,4) CASE(1,5) CASE(1,6) CASE(1,7)
		CASE(2,1) CASE(2,2) CASE(2,3) CASE(2,4) CASE(2,5) CASE(2,6) CASE(2,7)
		CASE(3,1) CASE(3,2) CASE(3,3) CASE(3,4) CASE(3,5) CASE(3,6) CASE(3,7)
		CASE(4,1) CASE(4,2) CASE(4,3) CASE(4,4) CASE(4,5) CASE(4,6) CASE(4,7)
		CASE(5,1) CASE(5,2) CASE(5,3) CASE(5,4) CASE(5,5) CASE(5,6) CASE(5,7)
		CASE(6,1) CASE(6,2) CASE(6,3) CASE(6,4) CASE(6,5) CASE(6,6) CASE(6,7)
		CASE(7,1) CASE(7,2) CASE(7,3) CASE(7,4) CASE(7,5) CASE(7,6) CASE(7,7)
#undef CASE
		// clang-format on

		std::cerr << "Unsupported size " << w << "x" << h << "\n";
	return 3;
}
