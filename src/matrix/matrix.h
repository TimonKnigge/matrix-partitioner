#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <utility>
#include <vector>

namespace mp {

constexpr int ROW = 0, COL = 1;

// A nonzero entry in a matrix.
struct entry {
	entry(int _r, int _c, int _ri, int _ci)
		:	pos{_r, _c}, r(pos[ROW]), c(pos[COL]),
			index{_ri, _ci}, ri(index[ROW]), ci(index[COL]) { }

	// The row and column in which the entry is positioned.
	int pos[2], &r = pos[ROW], &c = pos[COL];
	// The indices in the row and column list of the matrix.
	int index[2], &ri = index[ROW], &ci = index[COL];
};

struct matrix {
public:
	// The entries of the matrix, by row and column.
	std::vector<std::vector<entry>> adj[2],
		&rows = adj[ROW], &cols = adj[COL];
	// The number of rows and columns in the matrix.
	int R, C;
	// The number of nonzeros in the matrix.
	int NZ;
	// The maximal number of nonzeros in a single row or column.
	int Cmax;
	
	// Construct a matrix from a given set of nonzeros.
	matrix(int _R, int _C, std::vector<std::pair<int, int>> &&nonzeros);

	// Overload IO operator.
	friend std::ostream &operator<<(std::ostream &stream, const matrix &m);

	// Index the matrix by [row/col][index].
	const std::vector<std::vector<entry>> &operator[](int index) const;
};

}

#endif
