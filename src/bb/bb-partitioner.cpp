#include "./bb-partitioner.h"

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

	int max_partition_size = static_cast<int>(
		(1.0f + epsilon) * ((m.NZ + 1) / 2));
	if (2 * max_partition_size < m.NZ) {
		std::cerr << "No valid partitioning exists with this value of epsilon."
			<< std::endl;
		return false;
	}

	// Decide in which order to recurse on the rows/columns.
	std::vector<int> recursion_order(m.R + m.C, 0);
	std::iota(recursion_order.begin(), recursion_order.end(), 0);
	std::sort(recursion_order.begin(), recursion_order.end(),
		[&m](const int &l, const int &r) -> bool {
			return m[l].size() > m[r].size(); });

	// Partial partition.
	partial_partition pp(m, param, max_partition_size);

	// Optimal partition sofar.
	std::vector<status> optimal_status(m.R + m.C, status::unassigned);
	row.assign(m.R, status::unassigned);
	col.assign(m.C, status::unassigned);

	// Solve and store optimal status.
	int optimal_value = solve(recursion_order, pp, optimal_status);
	for (int r = 0; r < m.R; ++r) row[r] = optimal_status[r];
	for (int c = 0; c < m.C; ++c) col[c] = optimal_status[m.R + c];
	std::cerr << "Finished, found partition of size " << optimal_value
		<< std::endl;

	return true;
}

void bbpartitioner::recurse(int rc, status stat,
		std::stack<recursion_step> &call_stack, const partial_partition &pp) {
	if (!pp.can_assign(rc, stat)) return;
	call_stack.push(
		recursion_step{
			recursion_type::ascend,
			rc, pp.get_status(rc)});
	call_stack.push(
		recursion_step{
			recursion_type::descend,
			rc, stat});
}

void bbpartitioner::make_step(std::stack<recursion_step> &call_stack,
		size_t &current_rcs, std::vector<int> &rcs, partial_partition &pp,
		int upper_bound) {
	recursion_step step = call_stack.top();
	call_stack.pop();
	
	if (step.rt == recursion_type::descend) {
		pp.assign(step.rc, step.s);
		++current_rcs;
		if (current_rcs < rcs.size() && pp.lower_bound() < upper_bound) {
			recurse(rcs[current_rcs], status::cut, call_stack, pp);
			recurse(rcs[current_rcs], status::red, call_stack, pp);
			recurse(rcs[current_rcs], status::blue, call_stack, pp);
		}
	} else { // step.rt == recursion_type::ascend
		pp.undo(step.rc, step.s);
		--current_rcs;
	}
}

int bbpartitioner::solve(std::vector<int> &rcs, partial_partition &pp,
		std::vector<status> &optimal_status) {
	// `rcs` describes the order in which we pass through the rows/columns,
	// rcs[current_rcs] is the next row/column to branch on.
	size_t current_rcs = 0;

	// The current call stack. We manually only add `red` and `cut` (no `blue`,
	// this is to break symmetry.
	std::stack<recursion_step> call_stack;
	recurse(rcs[0], status::cut, call_stack, pp);
	recurse(rcs[0], status::red, call_stack, pp);

	// Now we manually apply recursion steps until the call stack is empty,
	// effectively traversing the B&B tree.
	int optimal_value = pp.m.R + pp.m.C + 2;
	while (!call_stack.empty()) {
		make_step(call_stack, current_rcs, rcs, pp, optimal_value);
		if (current_rcs == rcs.size()) {
			int cost = pp.lower_bound();
			if (optimal_value > cost) {
				optimal_value = cost;
				for (size_t i = 0; i < optimal_status.size(); ++i)
					optimal_status[i] = pp.get_status(i);
				std::cerr << "Improved solution found with cost " << cost
					<< std::endl;
			}
		}
	}

	return optimal_value;
}

}
