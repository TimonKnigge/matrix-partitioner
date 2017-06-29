#include "./matrix-util.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "./matrix.h"

matrix compress(const matrix &m) {
	std::unordered_map<int, int> newrow, newcol;
	for (int r = 0; r < m.R; ++r) {
		if (m[ROW][r].size() > 0) {
			newrow[r] = (int)newrow.size();
		}
	}
	for (int c = 0; c < m.C; ++c) {
		if (m[COL][c].size() > 0) {
			newcol[c] = (int)newcol.size();
		}
	}
	
	std::vector<std::pair<int, int>> new_nonzeros;
	for (int r = 0; r < m.R; ++r) {
		const auto &row = m[ROW][r];
		for (size_t i = 0; i < row.size(); ++i) {
			new_nonzeros.push_back({newrow[row[i].r], newcol[row[i].c]});
		}
	}
	return matrix((int)newrow.size(), (int)newcol.size(), new_nonzeros);
}
