#ifndef PARTITION_UTIL_H
#define PARTITION_UTIL_H

#include "../matrix/matrix-util.h"

namespace mp {

constexpr int RED = 0, BLUE = 1;
enum status {
	red = 0, blue = 1, cut = 2, unassigned = 3,
	partial_red = 4, partial_blue = 5, implicitly_cut = 6
};

inline status to_partial(status s) {
	if (s == status::red) return status::partial_red;
	if (s == status::blue) return status::partial_blue;
	return s;
}

inline status from_partial(status s) {
	if (s == status::partial_red) return status::red;
	if (s == status::partial_blue) return status::blue;
	return s;
}

inline bool is_partial(status s) {
	return s == status::partial_red || s == status::partial_blue;
}

inline status color_swap(status s) {
	if (s == status::red) return status::blue;
	if (s == status::blue) return status::red;
	if (s == status::partial_red) return status::partial_blue;
	if (s == status::partial_blue) return status::partial_red;
	return s;
}

inline int get_color(status s) {
	return s == status::red || s == status::partial_red ? RED : BLUE;
}

inline status color_to_status(int c) {
	return c == 0 ? status::red : status::blue;
}

inline status color_to_partial_status(int c) {
	return c == 0 ? status::partial_red : status::partial_blue;
}

}

#endif
