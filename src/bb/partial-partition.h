#ifndef PARTIAL_PARTITION_H
#define PARTIAL_PARTITION_H

#include <vector>

#include "bbparameters.h"
#include "../matrix/matrix.h"
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

  public:
	// The matrix partitioned.
	const matrix &m;

	explicit partial_partition(const matrix &_m, bbparameters _param,
		int _max_partition_size);

	// Whether or not a status can be assigned to the given row/column.
	bool can_assign(int rc, status s) const;

	// Assign a status to the given row/column.
	void assign(int rc, status s);

	// Undo the assignment of a status to the given row/column. Requires the
	// old status as a hint.
	void undo(int rc, status os);

	// Lower bound on the size of any extension of this partial partition.
	int lower_bound() const;

	// Status of the given row/column.
	status get_status(int rc) const;
};

}

#endif
