#ifndef INPUT_H
#define INPUT_H

#include <iostream>

#include "../matrix/matrix.h"

namespace mp {

// Reads a matrix in MM format.
matrix read_matrix(std::istream &stream);

}

#endif
