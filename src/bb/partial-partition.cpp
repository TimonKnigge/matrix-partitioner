#include "./partial-partition.h"

#include <algorithm>
#include <numeric>
#include <queue>

#include "../datastructures/min-heap.h"
#include "../io/output.h"

namespace mp {

partial_partition::partial_partition(const matrix &_m, bbparameters _param,
		int _max_partition_size) :
			param(_param),
			max_partition_size(_max_partition_size),
			stat(_m.R + _m.C, status::unassigned),
			vcg(_param.fb ? _m : matrix(_m.R, _m.C)),
			dfs_stack(_m.R + _m.C),
			dfs_index(_m.R + _m.C, -1),
			dfs_tree_size(_m.R + _m.C, 0),
			m(_m) {
	color_count[0].assign(m.R + m.C, 0);
	color_count[1].assign(m.R + m.C, 0);
}

bool partial_partition::can_assign(int rc, status s) const {
	switch (s) {
		case status::red: {
			int newred = ((int)m[rc].size() - color_count[RED][rc]);
			return (stat[rc] == status::unassigned
					|| stat[rc] == status::partial_red)
				&& partition_size[RED] + newred <= max_partition_size;
		}
		case status::blue: {
			int newblue = ((int)m[rc].size() - color_count[BLUE][rc]);
			return (stat[rc] == status::unassigned
					|| stat[rc] == status::partial_blue)
				&& partition_size[BLUE] + newblue <= max_partition_size;
		}
		case status::cut: {
			return stat[rc] == status::unassigned
				|| stat[rc] == status::partial_red
				|| stat[rc] == status::partial_blue
				|| stat[rc] == status::implicitly_cut;
		}
		default: {
			return false;
		}
	}
}

int partial_partition::assign(int rc, status s, int ub) {
	status os = stat[rc];

	// Adjust the simple packing sets if necessary. Assignment will certainly
	// remove 'partialness' so just remove the counts.
	if (param.pb || param.epb) {
		for (status cs : {status::partial_red, status::partial_blue}) {
			if (os != cs) continue;

			int color = get_color(cs);

			if (param.pb) {
				int free = m[rc].size() - color_count[color][rc];
				simple_packing_set[rc < m.R ? 0 : 1][color].remove(free);
			}
			if (param.epb) {
				std::unordered_set<int> &front = partition_front[color];
				front.erase(front.find(rc));
			}
		}
	}

	switch (s) {
		case status::cut: {
			if (os == status::implicitly_cut)
				--implicitly_cut;
			++cut;
			break;
		}
		case status::red:
		case status::blue: {
			int color = get_color(s);
			// Loop over all rows (or columns) that column (or row) `rc`
			// intersects, to update them.
			for (const entry &e : m[rc]) {
				status is = stat[e.rc];
				
				// If the intersecting row/column is already colored, there is
				// nothing to do.
				if (is == s) continue;

				// Check how many nonzeros are still free.
				int free = m[e.rc].size()
					- color_count[0][e.rc]
					- color_count[1][e.rc];
				// If using the simple packing bound, remove from packing set.
				if (param.pb && is_partial(is)) {
					int ocolor = get_color(is);
					simple_packing_set[e.rc < m.R ? 0 : 1][ocolor]
						.remove(free);
				}
				// Certainly this nonzero is no longer free.
				--free;

				// Otherwise, this entry is certainly not colored, so we color
				// it.
				++partition_size[color];
				++color_count[color][rc];
				++color_count[color][e.rc];

				// If the intersecting row/column is partially the other color,
				// we have implicitly cut it.
				if (is == to_partial(color_swap(s))) {
					stat[e.rc] = status::implicitly_cut;
					++implicitly_cut;
					if (param.epb) {
						std::unordered_set<int> &front
							= partition_front[get_color(color_swap(s))];
						front.erase(front.find(e.rc));
					}
				}

				// If the intersecting row/column is unassigned, it is now
				// partially colored.
				if (is == status::unassigned) {
					stat[e.rc] = color_to_partial_status(color);
					if (param.epb) {
						partition_front[color].insert(e.rc);
					}
				}

				// If the simple packing bound is enabled, add again.
				if (param.pb && is_partial(stat[e.rc])) {
					simple_packing_set[e.rc < m.R ? 0 : 1][color].add(free);
				}
			}
			break;
		}
		default: {
			std::cerr << "Error, trying to assign status " << s << std::endl;
			break;
		}
	}
	
	stat[rc] = s;

	// We start adjusting the lower bound to see if it exceeds ub.
	// If we go from implicitly cut to cut, there is no need to recompute
	// anything!
	if (!(os == status::implicitly_cut && s == status::cut))
		lower_bound_cache = incremental_lower_bound(rc, s, ub);
	return lower_bound_cache;
}

void partial_partition::undo(int rc, status os) {
	status s = stat[rc];

	switch (s) {
		case status::cut: {
			--cut;
			if (os == status::implicitly_cut)
				++implicitly_cut;
			break;
		}
		case status::red:
		case status::blue: {
			int color = get_color(s);
			// Loop over all rows (or columns) that column (or row) `rc`
			// intersects, to update them.
			for (const entry &e : m[rc]) {
				status is = stat[e.rc];
				
				// If the intersecting row/column is already colored, there is
				// nothing to do.
				if (is == s) continue;

				// Check how many nonzeros are still free.
				int free = m[e.rc].size()
					- color_count[0][e.rc]
					- color_count[1][e.rc];
				// If using the simple packing bound, remove from packing set.
				if (param.pb && is_partial(is)) {
					simple_packing_set[e.rc < m.R ? 0 : 1][color]
						.remove(free);
				}
				// Certainly this nonzero is free again.
				++free;

				// Otherwise, this entry was certainly not colored, so we
				// uncolor it.
				--partition_size[color];
				--color_count[color][rc];
				--color_count[color][e.rc];

				// If the intersecting row/column is implicitly cut but the
				// entry we are uncoloring was the last of its color, this
				// row/column becomes partially colored.
				if (is == status::implicitly_cut &&
						color_count[color][e.rc] == 0) {
					stat[e.rc] = to_partial(color_swap(s));
					--implicitly_cut;
					if (param.epb) {
						partition_front[get_color(color_swap(s))]
							.insert(e.rc);
					}
				}

				// If the intersecting row/column is partially colored but the
				// entry we are uncoloring was the last of its oclor, this row/
				// column becomes unassigned.
				if (is == to_partial(s) &&
						color_count[color][e.rc] == 0) {
					stat[e.rc] = status::unassigned;
					if (param.epb) {
						std::unordered_set<int> &front
							= partition_front[color];
						front.erase(front.find(e.rc));
					}
				}

				// If the simple packing bound is enabled, add again.
				if (param.pb && is_partial(stat[e.rc])) {
					int ocolor = get_color(stat[e.rc]);
					simple_packing_set[e.rc < m.R ? 0 : 1][ocolor].add(free);
				}
			}
			break;
		}
		default: {
			std::cerr << "Error: trying to undo incorrect status " << os
				<< std::endl;
			break;
		}
	}

	// Adjust the simple packing sets if necessary. Assignment will certainly
	// remove 'partialness' so just add the counts.
	if (param.pb || param.epb) {
		for (status cs : {status::partial_red, status::partial_blue}) {
			if (os != cs) continue;

			int color = get_color(cs);

			if (param.pb) {
				int free = m[rc].size() - color_count[color][rc];
				simple_packing_set[rc < m.R ? 0 : 1][color].add(free);
			}
			if (param.epb) {
				partition_front[color].insert(rc);
			}
		}
	}

	// Undo the flow bound.
	if (param.fb) {
		if (s == status::cut && os != status::implicitly_cut) {
			vcg.set_activity(rc, vertex_state::active);
		}
		if (s == status::red || s == status::blue) {
			vcg.set_activity(rc, vertex_state::active);
			for (const entry &e : m[rc]) {
				if (stat[e.rc] != status::implicitly_cut
						&& stat[e.rc] != status::cut
						&& vcg.get_activity(e.rc) == vertex_state::inactive) {
					vcg.set_activity(e.rc, vertex_state::active);
				}
			}
		}
	}

	stat[rc] = os;
}

int partial_partition::incremental_lower_bound(int rc, status s, int ub) {
	int lb_base = cut + implicitly_cut, lb_incr = 0;
	if (lb_base + lb_incr >= ub) return lb_base + lb_incr;

	// Simple packing bound.
	if (param.pb) {
		int pbv = 0;
		for (int roc = 0; roc < 2; ++roc) {
			for (int c = 0; c < 2; ++c) {
				int max_allowed = max_partition_size - partition_size[c];
				int available = simple_packing_set[roc][c].total_sum();
				if (max_allowed >= available) continue;

				// Cut as few rows/columns as necessary.
				int min_remove = available - max_allowed;
				simple_packing_set[roc][c].set_lower_bound(min_remove);
				pbv += simple_packing_set[roc][c]
					.get_minimum_packing_set_size();
			}
		}

		lb_incr = std::max(lb_incr, pbv);
		if (lb_base + lb_incr >= ub) return lb_base + lb_incr;
	}

	// Flow bound.
	if (param.fb) {
		// Most operations should speak for themselves.
		// We just assigned status s to vertex rc, so we now update the
		// state of rc and surrounding vertices in the flow graph.
		if (s == status::cut) {
			vcg.set_activity(rc, vertex_state::inactive);
		}
		if (s == status::red) {
			for (const entry &e : m[rc]) {
				if (stat[e.rc] == status::implicitly_cut) {
					vcg.set_activity(e.rc, vertex_state::inactive);
				}
			}
			vcg.set_activity(rc, vertex_state::source);
		}
		if (s == status::blue) {
			for (const entry &e : m[rc]) {
				if (stat[e.rc] == status::implicitly_cut) {
					vcg.set_activity(e.rc, vertex_state::inactive);
				}
			}
			vcg.set_activity(rc, vertex_state::sink);
		}

		lb_incr = std::max(lb_incr, vcg.get_minimum_vertex_cut());
		if (lb_base + lb_incr >= ub) return lb_base + lb_incr;
	}

	// Extended packing bound.
	if (param.epb) {
		// This bound is computed independently from the actual
		// vertex rc. TODO: perhaps we can gain some speedups by not
		// recomputing everything here -> only one side needs to be
		// recomputed?
		int epbv = 0;
		for (int c = 0; c < 2; ++c) {
			// We consider side c. We compute (the sizes of) a set of
			// trees springing from this side.
			std::vector<int> sizes = grow_trees(c);

			// Let's consider the amount of vertices we'll have to cut.
			// (note: this code is similar to the simple packing bound
			//  code)
			int max_allowed = max_partition_size - partition_size[c];
			int available = std::accumulate(sizes.begin(), sizes.end(), 0);
			if (max_allowed >= available) continue;

			// We will now start cutting the largest trees as necessary.
			int min_remove = available - max_allowed;
			sort(sizes.rbegin(), sizes.rend());
			for (size_t i = 0; i < sizes.size() && min_remove > 0; ++i) {
				min_remove -= sizes[i];
				epbv++;
			}
		}

		// We can request (in constant time) the flow paths and add
		// them to the extended packing bound, for a combined bound.
		lb_incr = std::max(lb_incr, epbv + vcg.get_minimum_vertex_cut());
		if (lb_base + lb_incr >= ub) return lb_base + lb_incr;
	}

	return lb_base + lb_incr;
}

std::vector<int> partial_partition::grow_trees(int c) {
	// This size of the partition springs from partition_size[c].
	// From each free vertex adjacent to it we grow a tree, trying
	// to accumulate an even set of subgraphs.
	
	// Reset/initialize all required datastructures.
	dfs_index.reset_all();
	dfs_tree_size.reset_all();
	mp::min_heap<int> dfs_heap;
	for (int rc : partition_front[c]) {
		if (vcg.is_free(rc)) {
			dfs_heap.push(key_value<int>{0, rc});
			dfs_stack[rc].push(rc);
			dfs_index.set((size_t)rc, 0);
		}
	}

	// Run all DFS's in parallel.
	while (!dfs_heap.empty()) {
		// First we grab the smallest tree, its stack, and the currently
		// active vertex in its DFS. If there are none, remove the tree.
		int rc = dfs_heap.top().value;
		std::stack<int> &st = dfs_stack[rc];
		if (st.empty()) {
			dfs_heap.pop();
			continue;
		}
		int u = st.top();

		// Consider expanding the next edge from u, if possible. If not, pop.
		int index = dfs_index.get((size_t)u);
		dfs_index.set((size_t)u, index+1);
		if (index == (int)m[u].size())
			st.pop();
		else {
			// End point of the edge, and reverse index.
			int v = m[u][index].rc;
			int vi = m[u][index].index;

			// There are several possible states for v. If v is already
			// colored red or blue we may NOT claim the edge. If it is
			// (implicitly) cut or part of a flow path we MAY claim the
			// edge. If it is partially red/blue we MAY OPTIONALLY claim
			// the edge FIRST. If it is unassigned and unclaimed we may
			// claim AND expand.
			mp::status vs = get_status(v);
			bool claimed_edge = false;
			if (vs == color_to_status(c)) {
				// Nothing, it is already red/blue and we can do nothing.
			} else if (vs == status::cut || vs == status::implicitly_cut
					|| !vcg.is_free(v)) {
				// We can claim the edge but not extend. This means
				// reinserting the stack as well.
				claimed_edge = true;
			} else {
				// Let's begin by checking the index of the target. If it
				// is -1 then we can expand. If it is not more than the
				// reverse index then we can at least claim the edge.
				int vsi = dfs_index.get((size_t)v);
				if (vsi <= vi)
					claimed_edge = true;

				if (vsi < 0) {
					// We can claim the vertex as well! Do it.
					dfs_index.set((size_t)v, 0);
					st.push(v);
				}
			}

			// If we claimed an edge, reinsert the stack.
			if (claimed_edge) {
				int sz = dfs_tree_size.get((size_t)rc);
				dfs_tree_size.set((size_t)rc, sz + 1);
				dfs_heap.pop();
				dfs_heap.push(key_value<int>{sz + 1, rc});
			}
		}
	}

	// Aggregate the input: check all partially c vertices on the
	// front and add their values if >0.
	std::vector<int> subgraph_sizes;
	for (int rc : partition_front[c]) {
		int sz = dfs_tree_size.get(rc);
		if (sz > 0)
			subgraph_sizes.push_back(sz);
	}
	return subgraph_sizes;
}

status partial_partition::get_status(int rc) const {
	return stat[rc];
}

int partial_partition::get_partition_size(int side) const {
	return partition_size[side];
}

int partial_partition::get_free_nonzeros(int rc) const {
	return (int)m[rc].size()
		- color_count[0][rc]
		- color_count[1][rc];
}

}
