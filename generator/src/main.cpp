#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "generator/GeneratorMinimax.hpp"

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
int run_bench_auto(int runs, const std::string& out, const std::string& fmt) {
    GeneratorMinimax<BB<W, H>, W, H> gen;
    gen.benchmark(runs);

    if (!out.empty()) {
        gen.saveTree(out, fmt);
    }
    return 0;
}

int main(int argc, char** argv) {
    int w = get_int_arg(argc, argv, "--w", 5);
    int h = get_int_arg(argc, argv, "--h", 5);
    int runs = get_int_arg(argc, argv, "--runs", 1);

    std::string out = get_str_arg(argc, argv, "--save-graph", "");
    std::string fmt = get_str_arg(argc, argv, "--format", "edges");

    // clang-format off
#define CASE(W,H) if (w == (W) && h == (H)) return run_bench_auto<(W),(H)>(runs, out, fmt);
    CASE(1,1) CASE(1,2) CASE(1,3) CASE(1,4) CASE(1,5) CASE(1,6)
    CASE(2,1) CASE(2,2) CASE(2,3) CASE(2,4) CASE(2,5) CASE(2,6)
    CASE(3,1) CASE(3,2) CASE(3,3) CASE(3,4) CASE(3,5) CASE(3,6)
    CASE(4,1) CASE(4,2) CASE(4,3) CASE(4,4) CASE(4,5) CASE(4,6)
    CASE(5,1) CASE(5,2) CASE(5,3) CASE(5,4) CASE(5,5) CASE(5,6)
    CASE(6,1) CASE(6,2) CASE(6,3) CASE(6,4) CASE(6,5) CASE(6,6)
#undef CASE
    // clang-format on

    std::cerr << "Unsupported size " << w << "x" << h << "\n";
    return 3;
}
