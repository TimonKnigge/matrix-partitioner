#include "./matrix.h"

#include <algorithm>

#include "../io/output.h"

namespace mp {

matrix::matrix(int _R, int _C, std::vector<std::pair<int, int>> &nonzeros) {
	std::cerr << "Constructed with " << R << ' ' << C << std::endl;
	R = _R;
	C = _C;
	NZ = (int)nonzeros.size();

	adj.resize(R + C);

	std::sort(nonzeros.begin(), nonzeros.end());
	for (std::pair<int, int> nz : nonzeros) {
		// Identifiers [0, R) are reserved for rows, [R, R+C) for the columns.
		int _r = nz.first, _c = R + nz.second;
		int _ri = (int)adj[_r].size();
		int _ci = (int)adj[_c].size();

		adj[_r].emplace_back(entry{_c, _ci});
		adj[_c].emplace_back(entry{_r, _ri});
		
		Cmax = std::max(Cmax, std::max(
			(int)adj[_r].size(),
			(int)adj[_c].size()));
	}
}

std::ostream &operator<<(std::ostream &stream, const matrix &m) {
	print_matrix(stream, m);
	return stream;
}

const std::vector<entry> &matrix::operator[](int index) const {
	return adj[index];
}

}
