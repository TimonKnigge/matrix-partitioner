#ifndef PACKING_SET_H
#define PACKING_SET_H

#include <map>

namespace mp {

// A datastructure for dynamically maintaining a 'packing set'.
// Specifically, this datastructure maintains a multiset of integers
// as well as a lowerbound C, and can return the size of the smallest
// subset of the multiset that sums to >= C.

class packing_set {

public:
	// Add/remove integers.
	void add(int c);
	void remove(int c);
	
	// Modify the lowerbound.
	void set_lower_bound(int nC);

	// Retrieve the size of the packing set.
	int get_minimum_packing_set_size() const;

private:
	int C = 0, ps_size = 0, ps_sum = 0;
	std::map<int, int> lower, upper;

	void migrate_from_lower_to_upper();
	void migrate_from_upper_to_lower();
};

}

#endif
