#include "./bbpartitioner.h"

#include <algorithm>

#include "../matrix/matrix.h"
#include "../matrix/matrix-util.h"

namespace mp {

bool bbpartitioner::partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, std::clock_t clocks) {
	bool valid;
	std::string error;
	std::tie(valid, error) = param.valid();
	if (!valid) {
		std::cerr << "Invalid parameters: " << error << std::endl;
		return false;
	}
	std::cerr << "Got matrix!!" << std::endl;
	return false;
}

}
