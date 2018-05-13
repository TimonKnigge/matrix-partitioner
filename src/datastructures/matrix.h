#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <utility>
#include <vector>

namespace mp {

// A nonzero entry in a matrix.
struct entry {
	// The other row/column identifier for this entry.
	int rc;
	// The index of the entry in that row or column.
	int index;
};

struct matrix {
public:
	// The entries of the matrix, by row and column.
	std::vector<std::vector<entry>> adj;

	// The number of rows and columns in the matrix.
	int R, C;
	// The number of nonzeros in the matrix.
	int NZ;
	// The maximal number of nonzeros in a single row or column.
	int Cmax;

	// Construct a matrix from a given set of nonzeros.
	matrix(int _R, int _C, std::vector<std::pair<int, int>> &nonzeros);

	// Overload IO operator.
	friend std::ostream &operator<<(std::ostream &stream, const matrix &m);

	// Index the matrix by [row/column identifier].
	const std::vector<entry> &operator[](int index) const;
};

}

#endif
