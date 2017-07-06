#include "./bbpartitioner.h"

#include <algorithm>

#include "../matrix/matrix.h"
#include "../matrix/matrix-util.h"

namespace mp {

bool bbpartitioner::partition(const matrix &m, std::vector<status> &row,
	std::vector<status> &col, float epsilon, std::clock_t clocks) {
	std::cerr << "Got matrix!!" << std::endl;
	return false;
}

}
