#include "./rvector.h"

#include <algorithm>
#include <limits>
#include <vector>

namespace mp {

template <class T>
T rvector<T>::get(size_t i) const {
	return current_time == timestamp[i] ? value[i] : def;
}

template <class T>
void rvector<T>::set(size_t i, T val) {
	value[i] = val;
	timestamp[i] = current_time;
}

template <class T>
void rvector<T>::reset_all() {
	++current_time;
	if (current_time == std::numeric_limits<size_t>::max()) {
		current_time = 0;
		std::fill(value.begin(), value.end(), def);
		std::fill(timestamp.begin(), timestamp.end(), 0);
	}
}

}
