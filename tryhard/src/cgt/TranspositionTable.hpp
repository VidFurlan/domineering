#pragma once
#include <cstdint>
#include <atomic>
#include <vector>
#include "cgt/ReplacePolicy.hpp"

// Warrasigma sigma tuf implementation
// https://rigtorp.se/spinlock/
struct Spinlock {
	std::atomic<bool> lock_ = {0};

	void lock() noexcept {
		for (;;) {
			// Optimistically assume the lock is free on the first try
			if (!lock_.exchange(true, std::memory_order_acquire)) {
				return;
			}
			// Wait for lock to be released without generating cache misses
			while (lock_.load(std::memory_order_relaxed)) {
				// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
				// hyper-threads
				__builtin_ia32_pause();
			}
		}
	}

	bool try_lock() noexcept {
		// First do a relaxed load to check if lock is free in order to prevent
		// unnecessary cache misses if someone does while(!try_lock())
		return !lock_.load(std::memory_order_relaxed) &&
			!lock_.exchange(true, std::memory_order_acquire);
	}

	void unlock() noexcept {
		lock_.store(false, std::memory_order_release);
	}
};

template <uint8_t W, uint8_t H>
struct TTEntry { // https://stackoverflow.com/questions/14707803/line-size-of-l1-and-l2-caches
		 // pazi da pod 64 byti zaradi L1 line sizov
	using BB = Bitboard<W, H>;

	BB mask{};
	uint8_t depth = 0;
	uint8_t value = 0;

	inline bool empty() const { return value == 0; }
};

template <uint8_t W, uint8_t H, class ReplacePolicy>
struct TTBucket {
	Spinlock lock;
	TTEntry<W, H> slots[ReplacePolicy::SLOTS];
};

template <uint8_t W, uint8_t H, class ReplacePolicy>
class TranspositionTable {
	private:
		std::vector<TTBucket<W, H, ReplacePolicy>> table;
		size_t mask;

	public:
		TranspositionTable(size_t mb_size) {
			size_t buckets = (mb_size * 1024 * 1024) / sizeof(TTBucket<W, H, ReplacePolicy>);
			size_t p2 = 1;
			while (p2 * 2 <= buckets) p2 *= 2;
			table = std::vector<TTBucket<W, H, ReplacePolicy>>(p2);
			mask = p2 - 1;
		}

		bool get(uint64_t hash, const Bitboard<W, H>& canonical_mask, TTEntry<W, H>& out) {
			size_t idx = hash & mask;
			auto &bucket = table[idx];
			bucket.lock.lock();
			for (size_t i = 0; i < ReplacePolicy::SLOTS; i++) {
				if (!bucket.slots[i].empty() && bucket.slots[i].mask == canonical_mask) {
					out = bucket.slots[i];
					bucket.lock.unlock();
					return true;
				}
			}
			bucket.lock.unlock();
			return false;
		}
		
		void store(uint64_t hash, const TTEntry<W, H>& entry) {
			size_t idx = hash & mask;
			auto &bucket = table[idx];
			bucket.lock.lock();
			ReplacePolicy::store(bucket.slots, entry);
			bucket.lock.unlock();
		}
};
