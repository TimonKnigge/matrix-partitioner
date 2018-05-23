#include "./packing-set.h"

#include <algorithm>

namespace mp {

void packing_set::add(int c) {
	// If c is zero just add it to the lower set.
	if (c == 0) {
		lower[c]++;
		return;
	}

	// Add c to the upper set.
	upper[c]++;
	ps_size++;
	ps_sum += c;

	// Adjust set.
	migrate_from_upper_to_lower();
}

void packing_set::remove(int c) {
	// Try the lower set first.
	auto it = lower.find(c);
	if (it != lower.end()) {
		--it->second;
		if (it->second == 0)
			lower.erase(it);
	} else {
		// We have to remove from the upper set.
		it = upper.find(c);
		--it->second;
		--ps_size;
		ps_sum -= c;
		if (it->second == 0)
			upper.erase(it);
		
		// Adjust set.
		migrate_from_lower_to_upper();
	}
}

void packing_set::set_lower_bound(int nC) {
	if (nC < C) {
		C = nC;
		migrate_from_upper_to_lower();
	} else if (nC > C) {
		C = nC;
		migrate_from_lower_to_upper();
	}
}

int packing_set::get_minimum_packing_set_size() const {
	return ps_size;
}

void packing_set::migrate_from_lower_to_upper() {
	while (ps_sum < C && !lower.empty()) {
		// Pick the largest quantity
		auto it = lower.end();
		--it;
		if (it->first <= 0) break;

		// Check how much we need to move at least.
		int mv = (C - ps_sum + it->first - 1) / it->first;

		// Move as much as possible.
		if (mv >= it->second) {
			upper[it->first] += it->second;
			ps_size += it->second;
			ps_sum += it->first * it->second;
			lower.erase(it);
		} else {
			upper[it->first] += mv;
			ps_size += mv;
			ps_sum += it->first * mv;
			it->second -= mv;
		}
	}
}

void packing_set::migrate_from_upper_to_lower() {
	while (ps_sum > C && !upper.empty()) {
		// Pick the smallest quantity
		auto it = upper.begin();
		if (ps_sum - it->first < C) break;

		// Check how much we can move.
		int mv = (ps_sum - C) / it->first;

		// Move as much we can.
		if (mv >= it->second) {
			lower[it->first] += it->second;
			ps_size -= it->second;
			ps_sum -= it->first * it->second;
			upper.erase(it);
		} else {
			lower[it->first] += mv;
			ps_size -= mv;
			ps_sum -= it->first * mv;
			it->second -= mv;
		}
	}
}

}
