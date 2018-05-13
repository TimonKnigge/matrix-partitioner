#include "./matrix-util.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "./matrix.h"

namespace mp {

matrix compress(const matrix &m, std::unordered_map<int, int> &idm) {
	int nR = 0, nC = 0;
	for (int id = 0; id < m.R + m.C; ++id) {
		if (m[id].size() > 0) {
			idm.emplace(id, (int)idm.size());
			(id < m.R ? nR : nC)++;
		}
	}

	std::vector<std::pair<int, int>> new_nonzeros;
	new_nonzeros.reserve(m.NZ);
	// We iterate only over the rows, this way we cover all nonzeros.
	for (int r = 0; r < m.R; ++r) {
		const auto &row = m[r];
		for (size_t i = 0; i < row.size(); ++i) {
			new_nonzeros.push_back({idm[r], idm[row[i].rc] - nR});
		}
	}
	return matrix(nR, nC, new_nonzeros);
}

}
