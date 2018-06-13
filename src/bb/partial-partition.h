#ifndef PARTIAL_PARTITION_H
#define PARTIAL_PARTITION_H

#include <iostream>
#include <stack>
#include <unordered_set>
#include <vector>

#include "bb-parameters.h"
#include "../datastructures/matrix.h"
#include "../datastructures/packing-set.h"
#include "../datastructures/rvector.h"
#include "../datastructures/vertex-cut-graph.h"
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

	// Packing sets to maintain the first packing bound for
	// rows and columns individually, for each color.
	// [R/C][R/B]
	mp::packing_set simple_packing_set[2][2];

	// A vertex cut graph for computing flows/minimal vertex cuts.
	mp::vertex_cut_graph vcg;

	// For the extended packing bound, we maintain a set of all partially
	// colored rows and columns.
	std::unordered_set<int> partition_front[2];

	// Also for the extended packing bound (for use during the DFS):
	// For each vertex a stack, a tree-size counter and an index
	// (explained in the DFS function).
	// The index vector is quickly resettable so it can be reused for each
	// DFS.
	std::vector<std::stack<int>> dfs_stack;
	mp::rvector<int> dfs_index, dfs_tree_size;

	// Rvectors for computing the distance of free vertices from the front.
	mp::rvector<int> dfront;

	// Compute a lower bound on the size of any extension of this partial
	// partition. Called by assign.
	int incremental_lower_bound(int rc, status s, int ub);

	// Cache the lower bound, sometimes we do not need to recompute anything.
	int lower_bound_cache = -1;

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
	// If the lower bound on the solution size is less than the upperbound it
	// will be returned exactly, otherwise we will return a lowerbound on the
	// lowerbound.
	int assign(int rc, status s, int ub);

	// Undo the assignment of a status to the given row/column. Requires the
	// old status as a hint. For example:
	//	if (pp.can_assign(rc, new_status)) {
	//		auto old_status = pp.get_status(rc);
	//		pp.assign(rc, new_status);
	//		<use pp, e.g. recurse>
	//		pp.undo(rc, old_status)
	//	}
	void undo(int rc, status os);

	// Grow a set of trees from the given side of the partition and return
	// their sizes. Based on the current state of the partition and vertex
	// cut graph.
	std::vector<int> grow_trees(int c);

	// Status of the given row/column.
	status get_status(int rc) const;

	// Retrieve the size of one side of the partition.
	int get_partition_size(int side) const;

	// How many free nonzeros there are in this row/column.
	int get_free_nonzeros(int rc) const;

	// How many actual rows/columns have been cut (no lowerbounding).
	int get_guaranteed_lower_bound() const;

	const mp::rvector<int> &get_dfront();

	// Friend for debugging.
	friend void print_ppmatrix(std::ostream &stream,
		const partial_partition &pp);
};

}

#endif
