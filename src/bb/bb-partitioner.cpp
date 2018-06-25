#include "./bb-partitioner.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <numeric>

#include "../datastructures/matrix-util.h"

namespace mp {

constexpr long long PERIOD_SMALL = 1000LL;

bool bbpartitioner::partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, long long tl) {
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
	double start = (double)clock();
	for (int U = param.U0, PU = 0;;
			PU = U, U = int(std::ceil(param.Uf * U))) {
		std::cerr << "Running with bound " << U << std::endl;
		optimal_value = solve(recursion_order, pp, optimal_status,
					tl > 0 ? start + tl * CLOCKS_PER_SEC : 0.0,
					PU, U);
		if (optimal_value < U) break;
	}

	for (int r = 0; r < m.R; ++r) row[r] = optimal_status[r];
	for (int c = 0; c < m.C; ++c) col[c] = optimal_status[m.R + c];
	if (optimal_value >= 0) {
		std::cerr << "Finished, found partition of volume " << optimal_value
			<< std::endl;
		std::cerr << "Used ~" << (std::ceil(clock()-start)/CLOCKS_PER_SEC)
			<< " seconds." << std::endl;
		return true;
	} else {
		std::cerr << "Out of time, last upperbound was " << -optimal_value
			<< std::endl;
		return false;
	}
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
	for (size_t i = current_rcs; i < rcs.size(); ++i) {
		int free = pp.get_free_nonzeros(rcs[i]);

		long long score = 1;
		score *= (long long)(1 + free);
		if (score > curscore) {
			curscore = score;
			std::swap(rcs[current_rcs], rcs[i]);
		}
	}

	return true;
}

void bbpartitioner::make_step(std::stack<recursion_step> &call_stack,
		size_t &current_rcs, std::vector<int> &rcs, partial_partition &pp,
		int &optimal_value, std::vector<status> &optimal_status) {
	recursion_step step = call_stack.top();
	call_stack.pop();
	
	int lb = -1;
	if (step.rt == recursion_type::descend) {
		// Make the assignment by taking a step in the bb tree.
		lb = pp.assign(step.rc, step.s, optimal_value);
		++current_rcs;

		std::cerr << "Did assignment of rc=" << step.rc <<", lb=" << lb << std::endl;
		print_ppmatrix(std::cerr, pp);

		// If we are in a leaf vertex, record and return.
		if (current_rcs == rcs.size() && lb < optimal_value) {
			optimal_value = lb;
			for (size_t i = 0; i < optimal_status.size(); ++i)
				optimal_status[i] = pp.get_status(i);
			std::cerr << "Improved solution found with cost " << lb
				<< std::endl;
			return;
		}

		// If we are not in a leaf vertex, we could try finding a
		// completion. To avoid redundant computations we only do
		// this in the root or after a vertex was cut.
		if ((current_rcs <= 1 || step.s == mp::status::cut) 
				&& lb < optimal_value) {
			partial_partition::completion stat
					= pp.find_completion(rcs, optimal_status);
			// If we find a completion we record it. Otherwise
			// if there doesn't exist one, we can safely increase
			// the lowerbound by 1.
			if (stat == partial_partition::completion::possible) {
				// Success, so fill in the remaining values and
				// record and return. Note: by returning before
				// we try branching again we make sure the rest
				// of this subtree is discarded.
				optimal_value = lb;
				for (size_t i = 0; i < current_rcs; ++i)
					optimal_status[rcs[i]] = pp.get_status(rcs[i]);
				std::cerr << "Improved solution found with cost " << lb
					<< " (by completion)." << std::endl;
				return;
			}
			if (stat == partial_partition::completion::impossible)
				lb += 1;
		}

		std::cerr << std::endl << "Will now find next" << std::endl;
		std::cerr << lb << ' ' << optimal_value << std::endl;

		// Try branching again.
		if (pick_next(current_rcs, rcs, pp, lb, optimal_value)) {
			std::cerr << "Next is " << rcs[current_rcs] << std::endl;
			// Recurse on the cut last (note that branches are executed on a
			// stack and thus in reverse order). Also, if lb + 1 == ub and this
			// vertex is NOT implicitly cut, there is no need to branch on
			// the cut.
			if (pp.get_status(rcs[current_rcs]) == mp::status::implicitly_cut
					|| pp.get_guaranteed_lower_bound() + 1 < optimal_value)
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
}

int bbpartitioner::solve(std::vector<int> &rcs, partial_partition &pp,
		std::vector<status> &optimal_status, double limit,
		int slb, int sub) {
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
	long long progress_counter = 0;
	while (!call_stack.empty()) {
		make_step(call_stack, current_rcs, rcs, pp, optimal_value,
				optimal_status);
		if (slb >= optimal_value)
			return optimal_value;

		progress_counter++;
		if (progress_counter % PERIOD_SMALL == 0LL) {
			if (limit > 1 && clock() > limit) {
				// Out of time.
				return -optimal_value;
			}
		}
	}

	return optimal_value;
}

}
