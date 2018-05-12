#ifndef RVECTOR_H
#define RVECTOR_H

#include <limits>
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

	// Functions are implemented here due to template logic.

	T get(size_t i) const {
		return current_time == timestamp[i] ? value[i] : def;
	}
	void set(size_t i, T val) {
		value[i] = val;
		timestamp[i] = current_time;
	}
	void reset_all() {
		++current_time;
		if (current_time == std::numeric_limits<size_t>::max()) {
			current_time = 0;
			std::fill(value.begin(), value.end(), def);
			std::fill(timestamp.begin(), timestamp.end(), 0);
		}
	}
};

}

#endif
