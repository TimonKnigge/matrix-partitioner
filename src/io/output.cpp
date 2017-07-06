#include "./output.h"

#include <iostream>

#include "../matrix/matrix.h"

namespace mp {

void print_matrix(std::ostream &stream, const matrix &m) {
	for (int r = 0; r < m.R; ++r) {
		size_t j = 0;
		for (int i = 0; i < m.C; ++i) {
			if (j < m[ROW][r].size() && m[ROW][r][j].c == i) {
				stream << '#';
				++j;
			} else	stream << '.';
		}
		stream << '\n';
	}
}

}
