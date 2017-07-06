#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include "./matrix.h"

namespace mp {

// Row/column compressed version of the given matrix. Returns a new matrix
// without empty rows or columns.
matrix compress(const matrix &m, std::vector<int> &rm, std::vector<int> &cm);

// Inverts a row/column flag.
inline int inv_rc(int row_or_column);

}

#endif
