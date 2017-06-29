#include "./output.h"

void print_matrix(ostream &stream, const matrix &m) {
	for (int r = 0; r < m.R; ++r) {
		size_t j = 0;
		for (int i = 0; i < m.C; ++i) {
			if (j < m[ROW].size() && m[ROW][j].c == i) {
				stream << '#';
				++j;
			} else	stream << '.';
		}
		stream << '\n';
	}
}
