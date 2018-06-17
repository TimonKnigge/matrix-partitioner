#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <vector>

#include "./partition-util.h"
#include "../datastructures/matrix.h"

namespace mp {

// An interface describing a matrix partitioner.
class partitioner {
  public:
	// Partition a given matrix, and store the assignment in row and col.
	// Returns true upon succes, false upon failure. Note: row and col may
	// still contain a partitioning when false is returned, in this case it
	// is the best partitioning found sofar.
	virtual bool partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, long long tl) = 0;

	virtual ~partitioner() {};
};

}

#endif
