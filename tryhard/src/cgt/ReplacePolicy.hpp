#pragma once

struct AlwaysReplace {
	static constexpr size_t SLOTS = 1;
	template <typename EntryArray, typename Entry>
		static void store(EntryArray& slots, const Entry& new_entry) {
			slots[0] = new_entry;
		}
};

struct DepthReplace {
	static constexpr size_t SLOTS = 2;
	template <typename EntryArray, typename Entry>
		static void store(EntryArray& slots, const Entry& new_entry) {
			if (slots[0].empty() || new_entry.depth >= slots[0].depth) slots[0] = new_entry;
			else slots[1] = new_entry;
		}
};

struct Depth2Replace {
	static constexpr size_t SLOTS = 3;
	template <typename EntryArray, typename Entry>
		static void store(EntryArray& slots, const Entry& new_entry) {
			if (slots[0].empty() || new_entry.depth >= slots[0].depth) slots[2] = slots[0], slots[0] = new_entry;
			else if (slots[1].empty() || new_entry.depth <= slots[1].depth) slots[1] = new_entry;
			else slots[2] = new_entry;
		}
};
