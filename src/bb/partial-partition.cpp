#include "./partial-partition.h"

#include <algorithm>
#include <numeric>
#include <queue>

#include "./subset-sum-computer.h"
#include "../datastructures/matrix-util.h"
#include "../datastructures/min-heap.h"
#include "../io/output.h"
#include "../partitioner/partition-util.h"

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
	color_count[RED].assign(m.R + m.C, 0);
	color_count[BLUE].assign(m.R + m.C, 0);
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
				simple_packing_set[rc < m.R ? ROWS : COLS][color]
					.remove(free);
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
				int free = get_free_nonzeros(e.rc);
				// If using the simple packing bound, remove from packing set.
				if (param.pb && is_partial(is)) {
					int ocolor = get_color(is);
					simple_packing_set[e.rc < m.R ? ROWS : COLS][ocolor]
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
					simple_packing_set[e.rc < m.R ? ROWS : COLS][color]
						.add(free);
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
				int free = get_free_nonzeros(e.rc);
				// If using the simple packing bound, remove from packing set.
				if (param.pb && is_partial(is)) {
					simple_packing_set[e.rc < m.R ? ROWS : COLS][color]
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
					simple_packing_set[e.rc < m.R ? ROWS : COLS][ocolor]
						.add(free);
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
				simple_packing_set[rc < m.R ? ROWS : COLS][color]
					.add(free);
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
		for (int roc : {ROWS, COLS}) {
			for (int c : {RED, BLUE}) {
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
		for (int c : {RED, BLUE}) {
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
		std::cerr << "Partial -> " << rc << std::endl;
		std::cerr << " state : " << (int)vcg.get_activity(rc) << std::endl;
		if (vcg.is_free(rc)) {
			std::cerr << " => pushing " << rc << std::endl;
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
		bool claimed_edge = false;
		while (!claimed_edge) {
			int index = dfs_index.get((size_t)u);
			if (index == (int)m[u].size()) {
				st.pop();
				break;
			}
			dfs_index.set((size_t)u, index+1);

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
			if (vs == mp::status::red || vs == mp::status::blue) {
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
		}

		// If we claimed an edge, reinsert the stack.
		if (claimed_edge) {
			int sz = dfs_tree_size.get((size_t)rc);
			dfs_tree_size.set((size_t)rc, sz + 1);
			dfs_heap.pop();
			dfs_heap.push(key_value<int>{sz + 1, rc});
		}
	}

	// Aggregate the input: check all partially c vertices on the
	// front and add their values if >0.
	std::vector<int> subgraph_sizes;
	std::cerr << "Subg sizes: ";
	for (int rc : partition_front[c]) {
		int sz = dfs_tree_size.get(rc);
		if (sz > 0) {
			subgraph_sizes.push_back(sz);
			std::cerr << ' ' << sz;
		}
	}
	std::cerr << std::endl;
	return subgraph_sizes;
}

partial_partition::completion partial_partition::find_completion(
		const std::vector<int> &rcs, std::vector<status> &assignment) {
	// If there exists a non-trivial vertex cut, we cannot extend this
	// partial partition without cutting extra rows or columns.
	if (vcg.get_minimum_vertex_cut() > 0)
		return partial_partition::completion::unknown;

	// To avoid redundant computations we ignore partial partitions with
	// implicitly cut rows and columns.
	if (implicitly_cut > 0)
		return partial_partition::completion::unknown;

	// If, through the packing bound, we already found that it is necessary
	// to cut more rows or columns to achieve a balanced partition, then we
	// also pass.
	if (lower_bound_cache > cut)
		return partial_partition::completion::unknown;

	std::cerr << "Passed preliminary checks." << std::endl;

	// Collect red, blue and free components.
	dfs_index.reset_all();
	std::stack<int> st;
	int completed_partition_size[2] = {partition_size[0], partition_size[1]};
	std::vector<int> comps, guaranteed[2];
	std::vector<std::vector<int>> comp_contents;
	std::cerr << "Init: {" << completed_partition_size[0] << ',';
	std::cerr << completed_partition_size[1] << "}" << std::endl;
	for (auto it = rcs.rbegin(); it != rcs.rend(); ++it) {
		int rc = *it;

		std::cerr << "Reached " << rc << std::endl;

		// Reached the assigned vertices, so we can break.
		mp::status stat = get_status(rc);
		if (stat == mp::status::red || stat == mp::status::blue
				|| stat == mp::status::cut)
			break;

	std::cerr << dfs_index.get((size_t)rc) << std::endl;

		// If we already visited this vertex we can continue.
		if (dfs_index.get((size_t)rc) >= 0)
			continue;

		std::cerr << "Starting search from " << rc << std::endl;

		// Explore the component through DFS.
		int side = -1, size = 0;
		st.push(rc);
		dfs_index.set((size_t)rc, 0);
		comp_contents.emplace_back();
		while (!st.empty()) {
			// We consider the edge from u to m[u][i]
			int u = st.top(), i = dfs_index.get((size_t)st.top());

			if (i == (int)m[u].size()) {
				st.pop();
				comp_contents.back().push_back(u);
				continue;
			}
			dfs_index.set((size_t)u, i+1);

			// End point of the edge (and reverse index).
			int v = m[u][i].rc;
			int vi = m[u][i].index;
			mp::status vs = get_status(v);

			if (vs == mp::status::red || vs == mp::status::blue) {
				// This edge is already colored, so we cannot claim it. But
				// we record its color.
				side = (vs == mp::status::red ? RED : BLUE);
			} else if (vs == status::cut) {
				// We can claim this edge but we can't extend, since this
				// vertex is cut.
				size += 1;
			} else {
				// Let's begin by checking the index of the target. If it
				// is -1 then we can expand. If it is not more than the
				// reverse index then we can at least claim the edge.
				int vsi = dfs_index.get((size_t)v);
				if (vsi <= vi)
					size += 1;
				if (vsi < 0) {
					dfs_index.set((size_t)v, 0);
					st.push(v);
				}
			}
		}

		std::cerr << "size=" << size << ", side="<<side << std::endl;

		// Record this component.
		if (side >= 0) {
			completed_partition_size[side] += size;
			guaranteed[side].insert(guaranteed[side].end(),
				comp_contents.back().begin(), comp_contents.back().end());
			comp_contents.pop_back();
		} else
			comps.push_back(size);
	}

	std::cerr << "Found " << comps.size() << " components." << std::endl;
	for (size_t i = 0; i < comps.size(); ++i) {
		std::cerr << comps[i] << " with";
		for (int j : comp_contents[i]) std::cerr << ' ' << j;
		std::cerr << std::endl;
	}
	std::cerr << "Post: {" << completed_partition_size[0] << ',';
	std::cerr << completed_partition_size[1] << "}" << std::endl;

	// Check if none of the partitions are too large already.
	if (completed_partition_size[RED] > max_partition_size
			|| completed_partition_size[BLUE] > max_partition_size)
		return partial_partition::completion::impossible;

TODO: if comps is 0 then just assign
TODO: if succesfull also do guaranteed

	// Try find a partition.
	std::vector<bool> part(comps.size(), false);
	if (mp::partition(comps,
			max_partition_size - completed_partition_size[RED],
			max_partition_size - completed_partition_size[BLUE], part)) {
		// Copy the partition into the 
		for (size_t i = 0; i < part.size(); ++i)
			for (int rc : comp_contents[i])
				assignment[rc] = (part[i] ? status::red : status::blue);
		return partial_partition::completion::possible;
	} else
		return partial_partition::completion::impossible;
}

status partial_partition::get_status(int rc) const {
	return stat[rc];
}

int partial_partition::get_partition_size(int side) const {
	return partition_size[side];
}

int partial_partition::get_free_nonzeros(int rc) const {
	return (int)m[rc].size()
		- color_count[RED][rc]
		- color_count[BLUE][rc];
}

int partial_partition::get_guaranteed_lower_bound() const {
	return cut;
}

const mp::vertex_cut_graph &partial_partition::get_vertex_cut_graph() const {
	return vcg;
}

}
