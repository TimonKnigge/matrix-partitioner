#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <queue>
#include <vector>

namespace mp {

template <class T>
struct key_value {
	int key;
	T value;
};

template <class T>
struct kv_comp {
	bool operator()(const key_value<T> &l, const key_value<T> &r) {
		return l.key > r.key;
	}
};

template <class T>
using min_heap = std::priority_queue<
	key_value<T>, std::vector<key_value<T>>, kv_comp<T>>;

}

#endif
