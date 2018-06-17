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

	// slb and sub are suggested lower and upperbounds. The solution will be
	// sought in [slb, sub). Returns -best-so-far when out of time. Limit
	// should be compared to clock() (i.e. set to clock()+t*CLOCKS_PER_SEC).
	int solve(std::vector<int> &rcs, partial_partition &pp,
		std::vector<status> &optimal_status, double limit,
		int slb = 0, int sub = -1);

	// Returns the lower bound after a descend, -1 for an ascend.
	int make_step(std::stack<recursion_step> &call_stack, size_t &current_rcs,
		std::vector<int> &rcs, partial_partition &pp, int upper_bound);

	void recurse(int rc, status stat, std::stack<recursion_step> &call_stack,
		const partial_partition &pp);

	// Pick the next vertex to branch on. Just moves it into position
	// rcs[current_rcs] so the algorithm will pick it up.
	bool pick_next(size_t &current_rcs, std::vector<int> &rcs,
		partial_partition &pp, int lower_bound, int upper_bound);

  public:
	bbpartitioner(bbparameters _param) : param(_param) { }

	virtual bool partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, long long tl);
};

}

#endif
