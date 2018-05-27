#include "./partial-partition.h"

#include "../io/output.h"

namespace mp {

partial_partition::partial_partition(const matrix &_m, bbparameters _param,
		int _max_partition_size) :
			param(_param),
			max_partition_size(_max_partition_size),
			stat(_m.R + _m.C, status::unassigned),
			vcg(_param.fb ? _m : matrix(_m.R, _m.C)),
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
	if (param.pb) {
		for (status cs : {status::partial_red, status::partial_blue}) {
			if (os != cs) continue;

			int color = get_color(cs);
			int free = m[rc].size() - color_count[color][rc];

			simple_packing_set[rc < m.R ? 0 : 1][color].remove(free);
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
				}

				// If the intersecting row/column is unassigned, it is now
				// partially colored.
				if (is == status::unassigned) {
					stat[e.rc] = color_to_partial_status(color);
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
	return incremental_lower_bound(rc, s, ub);
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
				}

				// If the intersecting row/column is partially colored but the
				// entry we are uncoloring was the last of its oclor, this row/
				// column becomes unassigned.
				if (is == to_partial(s) &&
						color_count[color][e.rc] == 0) {
					stat[e.rc] = status::unassigned;
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
	if (param.pb) {
		for (status cs : {status::partial_red, status::partial_blue}) {
			if (os != cs) continue;

			int color = get_color(cs);
			int free = m[rc].size() - color_count[color][rc];

			simple_packing_set[rc < m.R ? 0 : 1][color].add(free);
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

	return lb_base + lb_incr;
}

status partial_partition::get_status(int rc) const {
	return stat[rc];
}

int partial_partition::get_partition_size(int side) const {
	return partition_size[side];
}

}
