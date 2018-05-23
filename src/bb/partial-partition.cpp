#include "./partial-partition.h"

namespace mp {

partial_partition::partial_partition(const matrix &_m, bbparameters _param,
		int _max_partition_size) :
			param(_param),
			max_partition_size(_max_partition_size),
			stat(_m.R + _m.C, status::unassigned),
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

void partial_partition::assign(int rc, status s) {
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

	stat[rc] = os;
}

int partial_partition::lower_bound() {
	int lb = cut + implicitly_cut;

	// Simple packing bound.
	if (param.pb) {
		for (int rc = 0; rc < 2; ++rc) {
			for (int c = 0; c < 2; ++c) {
				int max_allowed = max_partition_size - partition_size[c];
				int available = simple_packing_set[rc][c].total_sum();
				if (max_allowed >= available) continue;

				// Cut as few rows/columns as necessary.
				int min_remove = available - max_allowed;
				simple_packing_set[rc][c].set_lower_bound(min_remove);
				lb += simple_packing_set[rc][c].get_minimum_packing_set_size();
			}
		}
	}

	return lb;
}

status partial_partition::get_status(int rc) const {
	return stat[rc];
}

int partial_partition::get_partition_size(int side) const {
	return partition_size[side];
}

}
