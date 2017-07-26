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

// TODO: implement partition size checking.
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
	if (os == status::implicitly_cut)
		--implicitly_cut;

	switch (s) {
		case status::cut: {
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
			}
			break;
		}
		default: {
			std::cerr << "Error: trying to undo incorrect status " << os
				<< std::endl;
			break;
		}
	}

	if (os == status::implicitly_cut)
		++implicitly_cut;
	stat[rc] = os;
}

int partial_partition::lower_bound() const {
	return cut + implicitly_cut;
}

status partial_partition::get_status(int rc) const {
	return stat[rc];
}

}
