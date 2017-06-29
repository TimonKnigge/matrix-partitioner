#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <utility>
#include <vector>

constexpr int ROW = 0, COL = 1;

// A nonzero entry in a matrix.
struct entry {
	// The row and column in which the entry is positioned.
	int pos[2], &r = pos[ROW], &c = pos[COL];
	// The indices in the row and column list of the matrix.
	int index[2], &ri = index[ROW], &ci = index[COL];
};

struct matrix {
public:
	// The entries of the matrix, by row and column.
	const std::vector<std::vector<entry>>> adj[2],
		&rows = adj[ROW], &cols = adj[COL];
	// The number of rows and columns in the matrix.
	const int R, C;
	// The number of nonzeros in the matrix.
	const int NZ;
	// The maximal number of nonzeros in a single row or column.
	const int Cmax;
	
	// Construct a matrix from a given set of nonzeros.
	explicit matrix(int _R, int _C,
		std::vector<std::pair<int, int>> &nonzeros);

	// Overload IO operator.
	friend std::ostream &operator<<(std::ostream &stream, const matrix &m)
		const;

	// Index the matrix by [row/col][index].
	const std::vector<entry> &operator[](const int index) const;
};

#endif
