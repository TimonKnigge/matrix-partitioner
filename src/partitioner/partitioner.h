#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <chrono>
#include <vector>

#include "./partition-util.h"
#include "../datastructures/matrix.h"

namespace mp {

// An interface describing a matrix partitioner.
class partitioner {
  public:
	// Partition a given matrix, and store the assignment in row and col.
	// Returns true upon succes, false upon failure.
	virtual bool partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, std::clock_t clocks) = 0;

	virtual ~partitioner() {};
};

}

#endif
