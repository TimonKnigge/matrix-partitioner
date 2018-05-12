#ifndef RVECTOR_H
#define RVECTOR_H

#include <vector>

namespace mp {

// This class represents a vector resettable in O(1) time.
template <class T>
class rvector {
private:
	// Default value, and the vector of values.
	T def;
	std::vector<T> value;

	// Current time and time of last set.
	size_t current_time = 0;
	std::vector<size_t> timestamp;

public:
	rvector(size_t N, T _def) {
		value.assign(N, def = _def);
		timestamp.assign(N, 0);
	}

	T get(size_t i) const;
	void set(size_t i, T val);
	void reset_all();
};

}

#endif
