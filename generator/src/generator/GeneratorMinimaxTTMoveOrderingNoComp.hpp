#pragma once
#include <array>
#include <algorithm>
#include <cstdlib>
#include "generator/GeneratorMinimaxTT.hpp"
#include "generator/utils/BitOps.hpp"

template <Bitboard T, std::uint8_t W, std::uint8_t H, class ReplacePolicy = AlwaysReplace> class GeneratorMinimaxTTMoveOrderingNoComp : public GeneratorMinimaxTT<T, W, H, ReplacePolicy> {
	public:
	using Base = GeneratorMinimaxTT<T, W, H, ReplacePolicy>;
	using typename Base::State;
	using Base::zobrist;
	using Base::compressor;

	private:
	static inline int distToCenter(int b) {
		int r = b / W;
		int c = b % W;
		int cr = H / 2;
		int cc = W / 2;
		return std::abs(r - cr) + std::abs(c - cc);
	}

	struct Cand { int b; int score; };

	bool solveOrdered(State s, uint64_t hash) {
		this->nodes++;

		//if ((this->nodes & ((1ll<<26)-1)) == ((1ll<<26)-1)) {
		//	std::cout << "Nodes: " << this->nodes << std::endl;
		//}

		bool cached;
		if (this->getMemo(s, hash, cached)) return cached;

		T empty = (~s.board) & Base::boardMask;

		std::array<Cand, W*H> cands{};
		int n = 0;

		if (s.turn == 0) {
			T anchors = empty & (empty >> W);
			if (!anchors) {
				this->setMemo(s, hash, false);
				return false;
			}
			T tmp = anchors;
			while (tmp) {
				int b = Base::ctzT(tmp);
				tmp &= (tmp - 1);
				cands[n++] = { b, distToCenter(b) };
			}

			std::sort(cands.begin(), cands.begin() + n,
					[](const Cand& x, const Cand& y){ return x.score < y.score; });

			for (int i = 0; i < n; ++i) {
				int b = cands[i].b;
				T a = (T{1} << b);
				T nextBoard = s.board | a | (a << W);

				uint64_t nhash = this->zobrist.hash(hash, (uint32_t)b);
				nhash = this->zobrist.hash(nhash, (uint32_t)(b + W));

				State ns;
				ns.board = nextBoard;
				ns.turn = 1;
				ns.depth = s.depth + 1;

				if (!solveOrdered(ns, nhash)) {
					this->setMemo(s, hash, true);
					return true;
				}
			}
		} else {
			T anchors = empty & ~Base::LAST_COL & (empty >> 1);
			if (!anchors) {
				this->setMemo(s, hash, false);
				return false;
			}
			T tmp = anchors;
			while (tmp) {
				int b = Base::ctzT(tmp);
				tmp &= (tmp - 1);
				cands[n++] = { b, distToCenter(b) };
			}

			std::sort(cands.begin(), cands.begin() + n,
					[](const Cand& x, const Cand& y){ return x.score < y.score; });

			for (int i = 0; i < n; ++i) {
				int b = cands[i].b;
				T a = (T{1} << b);
				T nextBoard = s.board | a | (a << 1);

				uint64_t nhash = this->zobrist.hash(hash, (uint32_t)b);
				nhash = this->zobrist.hash(nhash, (uint32_t)(b + 1));

				State ns;
				ns.board = nextBoard;
				ns.turn = 0;
				ns.depth = s.depth + 1;

				if (!solveOrdered(ns, nhash)) {
					this->setMemo(s, hash, true);
					return true;
				}
			}
		}

		this->setMemo(s, hash, false);
		return false;
	}

	public:
	void reset_tables() {
		this->TT0.assign(Base::TT_SIZE, State{});
		this->TT1.assign(Base::TT_SIZE, State{});
		this->nodes = 0;
		this->hits  = 0;
	}

	bool solve_subtree(State s, uint64_t hash) {
		return solveOrdered(s, hash);
	}

	uint64_t get_nodes() const { return this->nodes; }
	uint64_t get_hits()  const { return this->hits; }

	void run() override {
		this->TT0.assign(Base::TT_SIZE, State{});
		this->TT1.assign(Base::TT_SIZE, State{});
		this->nodes = 0;
		this->hits  = 0;

		State root;
		root.board = this->m_initial;
		root.turn  = 0;
		root.depth = 0;

		bool firstWins = solveOrdered(root, 0);

		std::cout << "Calls: " << this->nodes << "\n";
		std::cout << "Memo hits: " << this->hits << "\n";
		std::cout << "Wins: " << (firstWins ? "First" : "Second") << " player\n";
	}
};
