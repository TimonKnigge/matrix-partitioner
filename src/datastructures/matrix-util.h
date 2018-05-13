#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <unordered_map>

#include "./matrix.h"

namespace mp {

// Row/column compressed version of the given matrix. Returns a new matrix
// without empty rows or columns.
matrix compress(const matrix &m, std::unordered_map<int, int> &idm);

}

#endif
