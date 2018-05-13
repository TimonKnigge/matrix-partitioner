#ifndef BBPARTITIONER_H
#define BBPARTITIONER_H

#include <stack>

#include "./bb-parameters.h"
#include "../datastructures/matrix.h"
#include "./partial-partition.h"
#include "../partitioner/partitioner.h"
#include "../partitioner/partition-util.h"

namespace mp {

// Recursive operation.
enum recursion_type { descend, ascend };
struct recursion_step {
	recursion_type rt;
	int rc;
	status s;
};

// Branch and bound partitioner.
class bbpartitioner : public partitioner {
  private:
	bbparameters param;

	int solve(std::vector<int> &rcs, partial_partition &pp,
		std::vector<status> &optimal_status);

	void make_step(std::stack<recursion_step> &call_stack, size_t &current_rcs,
		std::vector<int> &rcs, partial_partition &pp, int upper_bound);

	void recurse(int rc, status stat, std::stack<recursion_step> &call_stack,
		const partial_partition &pp);

  public:
	bbpartitioner(bbparameters _param) : param(_param) { }

	virtual bool partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, std::clock_t clocks);
};

}

#endif
