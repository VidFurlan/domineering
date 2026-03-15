#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "generator/AGenerator.hpp"

// Worker req:
// State
// reset_tables()
// solve_subtree
// get_nodes()
// get_hits()
template <Bitboard T, std::uint8_t W, std::uint8_t H, class Worker>
class GeneratorMinimaxRootParallel : public AGenerator<T, W, H> {
    using State = typename Worker::State;

public:
    GeneratorMinimaxRootParallel(int threads = 1) : m_threads(threads) {}
    ~GeneratorMinimaxRootParallel() = default;

    void run() override {
        auto children = generate_root_children();

        if (children.empty()) {
            std::cout << "Calls: 1\n";
            std::cout << "Memo hits: 0\n";
            std::cout << "Wins: Second player\n";
            return;
        }

        std::atomic<bool> found_win = false;
        std::atomic<uint64_t> total_nodes = 0;
        std::atomic<uint64_t> total_hits  = 0;

#ifdef _OPENMP
        #pragma omp parallel for schedule(dynamic) num_threads(m_threads)
#endif
        for (int i = 0; i < (int)children.size(); ++i) {
            if (found_win.load(std::memory_order_relaxed)) continue;

            Worker worker;
            worker.reset_tables();

            auto [child_state, child_hash] = children[i];
            bool opp_wins = worker.solve_subtree(child_state, child_hash);

            total_nodes.fetch_add(worker.get_nodes(), std::memory_order_relaxed);
            total_hits.fetch_add(worker.get_hits(), std::memory_order_relaxed);

            // root is winning if there exists a child where opponent loses
            if (!opp_wins) {
                found_win.store(true, std::memory_order_relaxed);
            }
        }

        std::cout << "Calls: " << total_nodes.load() << "\n";
        std::cout << "Memo hits: " << total_hits.load() << "\n";
        std::cout << "Wins: " << (found_win.load() ? "First" : "Second") << " player\n";
    }

private:
    int m_threads;

    static constexpr int N = W * H;
    static constexpr T boardMask =
        (N == int(sizeof(T) * 8)) ? T(~T{0}) : (T{1} << N) - T{1};

    static constexpr T lastColMask() {
        T m = 0;
        for (int r = 0; r < H; ++r) {
            m |= (T{1} << (r * W + (W - 1)));
        }
        return m;
    }
    static constexpr T LAST_COL = lastColMask();

    static inline int ctzT_local(T x) {
        if constexpr (std::is_same_v<T, uint32_t>) return __builtin_ctz(x);
        else if constexpr (std::is_same_v<T, uint64_t>) return __builtin_ctzll(x);
        else {
            uint64_t lo = (uint64_t)x;
            if (lo) return __builtin_ctzll(lo);
            uint64_t hi = (uint64_t)(x >> 64);
            return 64 + __builtin_ctzll(hi);
        }
    }

    std::vector<std::pair<State, uint64_t>> generate_root_children() {
        std::vector<std::pair<State, uint64_t>> out;

        T root_board = this->m_initial;
        bool root_turn = 0; // first/vertical

        T empty = (~root_board) & boardMask;

        // local worker only for access to zobrist table
        Worker seed_worker;

        if (!root_turn) {
            T anchors = empty & (empty >> W);

            while (anchors) {
                int b = ctzT_local(anchors);
                anchors &= (anchors - 1);

                T a = (T{1} << b);
                T next_board = root_board | a | (a << W);

                uint64_t hash = 0;
                hash = seed_worker.zobrist.hash(hash, (uint32_t)b);
                hash = seed_worker.zobrist.hash(hash, (uint32_t)(b + W));

                State s{};
                s.board = next_board;
                s.turn  = 1;
                s.depth = 1;
                s.val   = 0;

                out.push_back({s, hash});
            }
        } else {
            T anchors = empty & ~LAST_COL & (empty >> 1);

            while (anchors) {
                int b = ctzT_local(anchors);
                anchors &= (anchors - 1);

                T a = (T{1} << b);
                T next_board = root_board | a | (a << 1);

                uint64_t hash = 0;
                hash = seed_worker.zobrist.hash(hash, (uint32_t)b);
                hash = seed_worker.zobrist.hash(hash, (uint32_t)(b + 1));

                State s{};
                s.board = next_board;
                s.turn  = 0;
                s.depth = 1;
                s.val   = 0;

                out.push_back({s, hash});
            }
        }

        return out;
    }
};
