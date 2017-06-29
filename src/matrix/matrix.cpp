#include "./matrix.h"

#include <algorithm>

explicit matrix::matrix(int _R, int _C,
	std::vector<std::pair<int, int>> &nonzeros) {
	R = _R;
	C = _C;
	NZ = (int)nonzeros.size();

	std::sort(nonzeros.begin(), nonzeros.end());
	for (std::pair<int, int> nz : nonzeros) {
		entry ent{nz.first, nz.second,
			(int)rows[nz.first ].size(),
			(int)cols[nz.second].size()};
		rows[ent.r].push_back(ent);
		cols[ent.c].push_back(ent);
		
		Cmax = std::max(Cmax, std:max(
			(int)rows[ent.r].size(),
			(int)cols[ent.c].size()));
	}
}

std::ostream &operator<<(std::ostream &stream, const matrix &m) const {
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
	return stream;
}

const sdt::vector<entry> &operator[](const int index) const {
	return adj[index];
}
