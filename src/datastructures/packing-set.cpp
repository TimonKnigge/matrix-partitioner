#include "./packing-set.h"

#include <algorithm>

namespace mp {

void packing_set::add(int c) {
	values[c]++;
	ps_size = -1;
	total += c;
}

void packing_set::remove(int c) {
	auto it = values.find(c);
	it->second--;
	if (it->second == 0)
		values.erase(it);
	ps_size = -1;
	total -= c;
}

void packing_set::set_lower_bound(int nC) {
	C = nC;
	ps_size = -1;
}

int packing_set::get_minimum_packing_set_size() {
	if (ps_size == -1)
		recompute();
	return ps_size;
}

void packing_set::recompute() {
	int ps = 0;
	ps_size = 0;
	for (auto it = values.rbegin(); ps < C && it != values.rend(); ++it) {
		int use = std::min(it->second, (C - ps + it->first - 1) / it->first);
		ps_size += use;
		ps += use * it->first;
	}
}

int packing_set::total_sum() const {
	return total;
}

}
