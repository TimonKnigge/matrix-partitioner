#include "./matrix.h"

#include <algorithm>

#include "../io/output.h"

namespace mp {

matrix::matrix(int _R, int _C,
	std::vector<std::pair<int, int>> &&nonzeros) {
	R = _R;
	C = _C;
	NZ = (int)nonzeros.size();

	// We need to do some early scanning to allocate the right amount of
	// memory. This is because we don't want to reallocate during
	// construction, since the entry's contain references which would be
	// lost.
	std::vector<int> rowsize(R, 0), colsize(C, 0);
	for (std::pair<int, int> nz : nonzeros)
		++rowsize[nz.first], ++colsize[nz.second];

	rows.resize(R);
	cols.resize(C);
	for (int r = 0; r < R; ++r) rows[r].reserve(rowsize[r]);
	for (int c = 0; c < C; ++c) cols[c].reserve(colsize[c]);

	std::sort(nonzeros.begin(), nonzeros.end());
	for (std::pair<int, int> nz : nonzeros) {
		int _r = nz.first, _c = nz.second;
		int _ri = (int)rows[nz.first ].size();
		int _ci = (int)cols[nz.second].size();

		rows[_r].emplace_back(_r, _c, _ri, _ci);
		cols[_c].emplace_back(_r, _c, _ri, _ci);
		
		Cmax = std::max(Cmax, std::max(
			(int)rows[_r].size(),
			(int)cols[_c].size()));
	}
}

std::ostream &operator<<(std::ostream &stream, const matrix &m) {
	print_matrix(stream, m);
	return stream;
}

const std::vector<std::vector<entry>> &matrix::operator[](int index) const {
	return adj[index];
}

}
