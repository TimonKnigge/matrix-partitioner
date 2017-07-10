#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include "./matrix.h"

namespace mp {

// Row/column compressed version of the given matrix. Returns a new matrix
// without empty rows or columns.
matrix compress(const matrix &m, std::vector<int> &idm);

}

#endif
