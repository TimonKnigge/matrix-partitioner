#include "./bb-partitioner.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "../datastructures/matrix-util.h"

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
	int optimal_value = 1;
	for (int U = param.U0, PU = -1;;
			PU = U, U = int(std::ceil(param.Uf * U))) {
		std::cerr << "Running with bound " << U << std::endl;
		optimal_value = solve(recursion_order, pp, optimal_status, PU, U);
		if (optimal_value < U) break;
	}

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

bool bbpartitioner::pick_next(size_t &current_rcs, std::vector<int> &rcs,
		partial_partition &pp, int lower_bound, int upper_bound) {
	if (current_rcs == rcs.size() || lower_bound >= upper_bound)
		return false;

	// First we branch on implicitly cut vertices, if they exist.
	for (size_t i = current_rcs; i < rcs.size(); ++i) {
		if (pp.get_status(rcs[i]) == status::implicitly_cut) {
			std::swap(rcs[current_rcs], rcs[i]);
			return true;
		}
	}

	// If not, we aggregate some statistics for each vertex and pick the
	// (subjectively) best option.
	long long curscore = -1LL;
	const mp::rvector<int> &dfront = pp.get_dfront();
	int mxdist = dfront.get((size_t)(pp.m.R + pp.m.C));
	for (size_t i = current_rcs; i < rcs.size(); ++i) {
		int free = pp.get_free_nonzeros(rcs[i]);
		int sbgw = pp.get_subgraph_weight(rcs[i]);
		int dfrt = dfront.get(rcs[i]); // Can be -1 !

		long long score = 1;
		score *= (long long)(1 + free);
		score *= (long long)(1 + free);
		score *= (long long)(1 + sbgw);
		score *= (long long)(1 + (dfrt < 0 ? mxdist + 1 : dfrt / 2));
		score *= (long long)(dfrt < 0 || dfrt % 2 == 1 ? 1LL : 10LL);
		if (score > curscore) {
			curscore = score;
			std::swap(rcs[current_rcs], rcs[i]);
		}
	}

	return true;
}

int bbpartitioner::make_step(std::stack<recursion_step> &call_stack,
		size_t &current_rcs, std::vector<int> &rcs, partial_partition &pp,
		int upper_bound) {
	recursion_step step = call_stack.top();
	call_stack.pop();
	
	int lb = -1;
	if (step.rt == recursion_type::descend) {
		lb = pp.assign(step.rc, step.s, upper_bound);

		// Try branching again.
		++current_rcs;
		if (pick_next(current_rcs, rcs, pp, lb, upper_bound)) {
			// Recurse on the cut last (note that branches are executed on a
			// stack and thus in reverse order). Also, if lb + 1 == ub and this
			// vertex is NOT implicitly cut, there is no need to branch on
			// the cut.
			if (pp.get_status(rcs[current_rcs]) == mp::status::implicitly_cut
					|| pp.get_guaranteed_lower_bound() + 1 < upper_bound)
				recurse(rcs[current_rcs], status::cut, call_stack, pp);

			// First branch on the smaller component.
			if (pp.get_partition_size(0) > pp.get_partition_size(1)) {
				recurse(rcs[current_rcs], status::red, call_stack, pp);
				recurse(rcs[current_rcs], status::blue, call_stack, pp);
			} else {
				if (pp.get_partition_size(0) > 0)
					recurse(rcs[current_rcs], status::blue, call_stack, pp);
				recurse(rcs[current_rcs], status::red, call_stack, pp);
				// Note that we only recurse into 'blue' if a red row/column
				// already exists, this is to break symmetry.
			}
		}
	} else { // step.rt == recursion_type::ascend
		pp.undo(step.rc, step.s);
		--current_rcs;
	}

	return lb;
}

int bbpartitioner::solve(std::vector<int> &rcs, partial_partition &pp,
		std::vector<status> &optimal_status, int slb, int sub) {
	// `rcs` describes the order in which we pass through the rows/columns,
	// rcs[current_rcs] is the next row/column to branch on.
	size_t current_rcs = 0;

	// The current call stack. We manually only add `red` and `cut` (no `blue`,
	// this is to break symmetry.
	std::stack<recursion_step> call_stack;
	recurse(rcs[0], status::cut, call_stack, pp);
	recurse(rcs[0], status::red, call_stack, pp);

	// Pick a good starting bound.
	int optimal_value = std::min(pp.m.R, pp.m.C) + 2;
	if (sub > 0 && sub < optimal_value)
		optimal_value = sub;

	// Now we manually apply recursion steps until the call stack is empty,
	// effectively traversing the B&B tree.
	while (!call_stack.empty()) {
		int lb = make_step(call_stack, current_rcs, rcs, pp, optimal_value);
		if (current_rcs == rcs.size()) {
			if (optimal_value > lb) {
				optimal_value = lb;
				for (size_t i = 0; i < optimal_status.size(); ++i)
					optimal_status[i] = pp.get_status(i);
				std::cerr << "Improved solution found with cost " << lb
					<< std::endl;
				// If we are already hitting the suggested lower bound we
				// can stop.
				if (slb >= optimal_value) {
					return optimal_value;
				}
			}
		}
	}

	return optimal_value;
}

}
