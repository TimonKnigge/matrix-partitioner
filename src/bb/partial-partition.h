#ifndef PARTIAL_PARTITION_H
#define PARTIAL_PARTITION_H

#include <vector>

#include "bb-parameters.h"
#include "../datastructures/matrix.h"
#include "../partitioner/partition-util.h"

namespace mp {

class partial_partition {
  private:
	// Parameters describing the partitioning method.
	bbparameters param;

	// Current number of cut & implicitly cut columns.
	int cut = 0, implicitly_cut = 0;

	// Size of the partitions.
	int partition_size[2] = {0, 0}, max_partition_size;
	
	// Status of each row/column.
	std::vector<status> stat;

	// Number of nonzeros of each color in each row/column.
	std::vector<int> color_count[2];

  public:
	// The matrix partitioned.
	const matrix &m;

	partial_partition(const matrix &_m, bbparameters _param,
		int _max_partition_size);

	// Whether or not a status can be assigned to the given row/column.
	bool can_assign(int rc, status s) const;

	// Assign a status to the given row/column. Note that you can not fill in
	// an arbitrary status for the second argument. Only status changes
	// naturally occurring during B&B are supported. See the implementation of
	// can_assign for details.
	void assign(int rc, status s);

	// Undo the assignment of a status to the given row/column. Requires the
	// old status as a hint. For example:
	//	if (pp.can_assign(rc, new_status)) {
	//		auto old_status = pp.get_status(rc);
	//		pp.assign(rc, new_status);
	//		<use pp, e.g. recurse>
	//		pp.undo(rc, old_status)
	//	}
	void undo(int rc, status os);

	// Lower bound on the size of any extension of this partial partition.
	int lower_bound() const;

	// Status of the given row/column.
	status get_status(int rc) const;

	// Retrieve the size of one side of the partition.
	int get_partition_size(int side) const;
};

}

#endif
